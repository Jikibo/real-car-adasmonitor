#include "dms_monitor.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr float kPi = 3.14159265358979323846f;

cv::Rect clampRectToImage(const cv::Rect& r, const cv::Size& size) {
    cv::Rect img_rect(0, 0, size.width, size.height);
    return r & img_rect;
}
}  // namespace

DMSMonitor::DMSMonitor(const std::string& model_dir) {
    const std::string prototxt = model_dir + "/deploy.prototxt";
    const std::string caffemodel = model_dir + "/res10_300x300_ssd_iter_140000.caffemodel";
    const std::string eye_model = model_dir + "/haarcascade_eye.xml";

    face_net_ = cv::dnn::readNetFromCaffe(prototxt, caffemodel);
    eye_cascade_.load(eye_model);
}

bool DMSMonitor::isReady() const {
    return !face_net_.empty() && !eye_cascade_.empty();
}

DriverState DMSMonitor::analyze(const cv::Mat& frame) {
    DriverState state;

    if (frame.empty() || !isReady()) {
        return state;
    }

    float confidence = 0.0f;
    state.face_rect = detectFace(frame, &confidence);
    state.face_detected = state.face_rect.area() > 0;

    if (!state.face_detected) {
        state.eye_openness = 0.0f;
        state.eyes_open = false;
        state.head_turn_deg = 0.0f;
        state.looking_forward = false;
        state.alert_drowsy = false;
        state.alert_distracted = true;
        updateEyeHistory(false);
        return state;
    }

    state.eye_openness = estimateEyeOpenness(frame, state.face_rect);
    state.eyes_open = state.eye_openness >= 0.2f;

    state.head_turn_deg = estimateHeadTurn(frame, state.face_rect);
    state.looking_forward = std::abs(state.head_turn_deg) <= looking_forward_threshold_deg_;

    updateEyeHistory(state.eyes_open);
    state.alert_drowsy = isDrowsy();
    state.alert_distracted = !state.looking_forward;

    return state;
}

cv::Rect DMSMonitor::detectFace(const cv::Mat& frame, float* confidence) {
    if (confidence) {
        *confidence = 0.0f;
    }

    cv::Mat blob = cv::dnn::blobFromImage(
        frame,
        1.0,
        cv::Size(300, 300),
        cv::Scalar(104.0, 117.0, 123.0),
        false,
        false
    );

    face_net_.setInput(blob);
    cv::Mat detections = face_net_.forward();

    cv::Rect best_rect;
    float best_conf = 0.0f;

    const int num_detections = detections.size[2];
    for (int i = 0; i < num_detections; ++i) {
        const float* det = detections.ptr<float>(0, 0, i);
        const float conf = det[2];

        if (conf <= face_conf_threshold_ || conf <= best_conf) {
            continue;
        }

        int x1 = static_cast<int>(det[3] * frame.cols);
        int y1 = static_cast<int>(det[4] * frame.rows);
        int x2 = static_cast<int>(det[5] * frame.cols);
        int y2 = static_cast<int>(det[6] * frame.rows);

        cv::Rect candidate(x1, y1, x2 - x1, y2 - y1);
        candidate = clampRectToImage(candidate, frame.size());

        if (candidate.area() > 0) {
            best_rect = candidate;
            best_conf = conf;
        }
    }

    if (confidence) {
        *confidence = best_conf;
    }

    return best_rect;
}

float DMSMonitor::estimateEyeOpenness(
    const cv::Mat& frame,
    const cv::Rect& face_rect
) {
    cv::Rect face = clampRectToImage(face_rect, frame.size());
    if (face.area() <= 0) {
        return 0.0f;
    }

    cv::Mat face_roi = frame(face);
    cv::Mat gray;
    cv::cvtColor(face_roi, gray, cv::COLOR_BGR2GRAY);
    cv::equalizeHist(gray, gray);

    cv::Rect upper_half(0, 0, gray.cols, std::max(1, gray.rows / 2));
    cv::Mat upper = gray(upper_half);

    std::vector<cv::Rect> eyes;
    eye_cascade_.detectMultiScale(
        upper,
        eyes,
        1.1,
        3,
        0,
        cv::Size(16, 16)
    );

    const int eye_count = static_cast<int>(std::min<size_t>(eyes.size(), 2));
    float openness = static_cast<float>(eye_count) / 2.0f;

    if (openness < 0.0f) openness = 0.0f;
    if (openness > 1.0f) openness = 1.0f;

    return openness;
}

float DMSMonitor::estimateHeadTurn(const cv::Mat& frame, const cv::Rect& face_rect) const {
    cv::Rect face = clampRectToImage(face_rect, frame.size());
    if (face.area() <= 0 || frame.cols <= 0) {
        return 0.0f;
    }

    const float face_center_x = face.x + face.width * 0.5f;
    const float frame_center_x = frame.cols * 0.5f;
    const float dx = face_center_x - frame_center_x;

    const float angle_rad = std::atan2(dx, frame.cols * 0.75f);
    const float angle_deg = angle_rad * 180.0f / kPi;

    return angle_deg;
}

void DMSMonitor::updateEyeHistory(bool eyes_open) {
    eye_history_.push_back(eyes_open);

    while (static_cast<int>(eye_history_.size()) > kHistorySize) {
        eye_history_.pop_front();
    }
}

bool DMSMonitor::isDrowsy() const {
    if (static_cast<int>(eye_history_.size()) < kHistorySize) {
        return false;
    }

    int closed_count = 0;
    for (bool open : eye_history_) {
        if (!open) {
            ++closed_count;
        }
    }

    return closed_count >= 30;
}
