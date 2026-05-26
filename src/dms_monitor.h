#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

#include <deque>
#include <string>

struct DriverState {
    bool face_detected = false;
    bool eyes_open = false;
    bool looking_forward = true;
    float eye_openness = 0.0f;
    float head_turn_deg = 0.0f;
    bool alert_drowsy = false;
    bool alert_distracted = false;
    cv::Rect face_rect;
};

class DMSMonitor {
public:
    explicit DMSMonitor(const std::string& model_dir = "../models");

    DriverState analyze(const cv::Mat& frame);

    bool isReady() const;

private:
    cv::Rect detectFace(const cv::Mat& frame, float* confidence = nullptr);
    float estimateEyeOpenness(const cv::Mat& frame, const cv::Rect& face_rect);
    float estimateHeadTurn(const cv::Mat& frame, const cv::Rect& face_rect) const;

    void updateEyeHistory(bool eyes_open);
    bool isDrowsy() const;

private:
    cv::dnn::Net face_net_;
    cv::CascadeClassifier eye_cascade_;
    std::deque<bool> eye_history_;

    float face_conf_threshold_ = 0.5f;
    float looking_forward_threshold_deg_ = 15.0f;

    static constexpr int kHistorySize = 45;
};
