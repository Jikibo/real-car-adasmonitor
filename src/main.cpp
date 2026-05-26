#include <atomic>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <thread>
#include <filesystem>

#include <opencv2/opencv.hpp>

#include "obd_parser.h"
#include "onnx_classifier.h"
#include "dashboard.h"
#include "dms_monitor.h"
#include "shared_state.h"
#include "dms_hud.h"

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
        } catch (const std::exception&) {
            state.running.store(false);
            break;
        }
    }
}

int main() {
    try {
        std::filesystem::create_directories("../output");

        OBDParser parser("../data/obd_data.csv");
        if (parser.load() < 0) {
            std::cerr << "Failed to load OBD data\n";
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

        const std::string windowName = "RealCarMonitor";
        cv::namedWindow(windowName, cv::WINDOW_NORMAL);

        cv::VideoWriter writer(
            "../output/result_situation2.mp4",
            cv::VideoWriter::fourcc('a', 'v', 'c', '1'),
            30.0,
            cv::Size(1280, 480)
        );

        std::ofstream logFile("../output/dms_alerts.log", std::ios::app);
        bool paused = false;

        auto logAlert = [&](const DriverState& ds, const ClassificationResult& cr) {
            if (!logFile.is_open()) return;
            if (!(ds.alert_drowsy || ds.alert_distracted || cr.label == 2)) return;

            auto now = std::chrono::system_clock::now();
            std::time_t t = std::chrono::system_clock::to_time_t(now);

            logFile << std::put_time(std::localtime(&t), "%F %T")
                    << " | drowsy=" << ds.alert_drowsy
                    << " distracted=" << ds.alert_distracted
                    << " aggressive=" << (cr.label == 2)
                    << '\n';
        };

        while (state.running.load()) {
            try {
                if (cv::getWindowProperty(windowName, cv::WND_PROP_VISIBLE) < 1) {
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

            cv::Mat canvas(480, 1280, CV_8UC3, cv::Scalar(0, 0, 0));
            cv::Mat left = canvas(cv::Rect(0, 0, 640, 480));
            cv::Mat right = canvas(cv::Rect(640, 0, 640, 480));

            dashboard.draw(
                left,
                static_cast<float>(record.speed_kmh),
                static_cast<float>(record.engine_rpm),
                static_cast<float>(record.coolant_temp),
                static_cast<float>(record.fuel_level),
                static_cast<float>(record.throttle_pos),
                OBDParser::intToLabel(result.label)
            );

            cv::resize(frame, right, right.size());

            cv::putText(right, driver_state.face_detected ? "FACE: DETECTED" : "FACE: NOT DETECTED",
                        {20, 30}, cv::FONT_HERSHEY_SIMPLEX, 0.7,
                        driver_state.face_detected ? cv::Scalar(0,255,0) : cv::Scalar(0,0,255), 2, cv::LINE_AA);

            cv::putText(right, driver_state.eyes_open ? "EYES: OPEN" : "EYES: CLOSED",
                        {20, 65}, cv::FONT_HERSHEY_SIMPLEX, 0.7,
                        driver_state.eyes_open ? cv::Scalar(0,255,0) : cv::Scalar(0,0,255), 2, cv::LINE_AA);

            cv::putText(right, driver_state.looking_forward ? "HEAD: FORWARD" : "HEAD: TURNED",
                        {20, 100}, cv::FONT_HERSHEY_SIMPLEX, 0.7,
                        driver_state.looking_forward ? cv::Scalar(0,255,0) : cv::Scalar(0,0,255), 2, cv::LINE_AA);

            logAlert(driver_state, result);
            writer.write(canvas);

            cv::imshow(windowName, canvas);

            int key = paused ? cv::waitKey(0) : cv::waitKey(1);
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

        cv::destroyAllWindows();
        state.running.store(false);
        obd_thread.join();

        std::cout << "Время работы системы: завершено\n";
        std::cout << "Обработано записей OBD: " << parser.size() << '\n';
        std::cout << "Количество алертов всего: " << state.alert_count << '\n';

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << '\n';
        return -1;
    }
}