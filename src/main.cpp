#include <atomic>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <thread>

#include <opencv2/opencv.hpp>

#include "dashboard.h"
#include "dms_hud.h"
#include "dms_monitor.h"
#include "obd_parser.h"
#include "onnx_classifier.h"
#include "shared_state.h"

namespace fs = std::filesystem;

void obd_thread_function(OBDParser& parser, ONNXClassifier& classifier, SharedState& state) {
    const std::size_t total = parser.size();
    if (total == 0) {
        state.running.store(false);
        return;
    }

    std::size_t index = 0;

    while (state.running.load()) {
        try {
            const OBDRecord& record = parser.getRecord(static_cast<int>(index));
            ClassificationResult result = classifier.classify(record);

            {
                std::lock_guard<std::mutex> lock(state.mtx);
                state.current_record = record;
                state.classification_result = result;
            }

            index = (index + 1) % total;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        } catch (const std::exception& e) {
            std::cerr << "OBD thread error: " << e.what() << '\n';
            state.running.store(false);
            break;
        }
    }
}

int main() {
    try {
        fs::create_directories("../output");

        OBDParser parser("../data/obd_data.csv");
        if (parser.load() < 0) {
            std::cerr << "Failed to load OBD CSV\n";
            return -1;
        }

        ONNXClassifier classifier(
            "../models/driver_classifier.onnx",
            "../models/normalization_params.json"
        );

        DMSMonitor dms_monitor("../models");
        Dashboard dashboard;
        DMSHUD dms_hud;
        SharedState state;

        std::thread obd_thread(
            obd_thread_function,
            std::ref(parser),
            std::ref(classifier),
            std::ref(state)
        );

        cv::VideoCapture cap(0, cv::CAP_DSHOW);
        if (!cap.isOpened()) {
            std::cerr << "Cannot open webcam\n";
            state.running.store(false);
            obd_thread.join();
            return -1;
        }

        const int width = 1280;
        const int height = 480;
        const std::string window_name = "RealCarMonitor";

        cv::namedWindow(window_name, cv::WINDOW_NORMAL);

        cv::VideoWriter writer(
            "../output/result_situation2.mp4",
            cv::VideoWriter::fourcc('m', 'p', '4', 'v'),
            30.0,
            cv::Size(width, height)
        );

        if (!writer.isOpened()) {
            std::cerr << "Warning: video writer not opened\n";
        }

        std::ofstream log_file("../output/dms_alerts.log", std::ios::app);

        auto log_alert = [&](const DriverState& ds, const ClassificationResult& cr) {
            if (!(ds.alert_drowsy || ds.alert_distracted || cr.label == 2)) {
                return;
            }

            state.alert_count++;

            if (log_file.is_open()) {
                const auto now = std::chrono::system_clock::now();
                const std::time_t now_c = std::chrono::system_clock::to_time_t(now);

                log_file << std::put_time(std::localtime(&now_c), "%F %T")
                         << " | drowsy=" << ds.alert_drowsy
                         << " distracted=" << ds.alert_distracted
                         << " aggressive=" << (cr.label == 2)
                         << '\n';
                log_file.flush();
            }
        };

        auto start_time = std::chrono::steady_clock::now();
        bool paused = false;

        while (state.running.load()) {
            try {
                if (cv::getWindowProperty(window_name, cv::WND_PROP_VISIBLE) < 1) {
                    state.running.store(false);
                    break;
                }
            } catch (...) {
                state.running.store(false);
                break;
            }

            cv::Mat frame;
            cap >> frame;
            if (frame.empty()) {
                std::cerr << "Empty camera frame\n";
                break;
            }

            DriverState driver_state = dms_monitor.analyze(frame);

            OBDRecord record;
            ClassificationResult result;
            {
                std::lock_guard<std::mutex> lock(state.mtx);
                record = state.current_record;
                result = state.classification_result;
            }

            cv::Mat canvas(height, width, CV_8UC3, cv::Scalar(0, 0, 0));
            cv::Mat left = canvas(cv::Rect(0, 0, width / 2, height));
            cv::Mat right = canvas(cv::Rect(width / 2, 0, width / 2, height));

            dashboard.draw(
                left,
                static_cast<float>(record.speed_kmh),
                static_cast<float>(record.engine_rpm),
                static_cast<float>(record.coolant_temp),
                static_cast<float>(record.fuel_level),
                static_cast<float>(record.throttle_pos),
                OBDParser::intToLabel(result.label)
            );

            cv::Mat hud_frame = dms_hud.draw(frame, driver_state);
            if (!hud_frame.empty()) {
                if (hud_frame.size() == right.size()) {
                    hud_frame.copyTo(right);
                } else {
                    cv::resize(hud_frame, right, right.size(), 0.0, 0.0, cv::INTER_LINEAR);
                }
            }

            log_alert(driver_state, result);

            if (writer.isOpened()) {
                writer.write(canvas);
            }

            cv::imshow(window_name, canvas);

            const int delay = paused ? 0 : 1;
            int key = cv::waitKey(delay);

            if (key == 'q' || key == 'Q' || key == 27) {
                state.running.store(false);
                break;
            }
            if (key == ' ') {
                paused = !paused;
            }
            if (key == 's' || key == 'S') {
                cv::imwrite("../output/screenshot.png", canvas);
            }
        }

        state.running.store(false);
        obd_thread.join();

        cv::destroyAllWindows();

        const auto end_time = std::chrono::steady_clock::now();
        const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();

        std::cout << "System runtime: " << seconds << " sec\n";
        std::cout << "Processed OBD records: " << parser.size() << '\n';
        std::cout << "Total alerts: " << state.alert_count << '\n';

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << '\n';
        return -1;
    }
}
