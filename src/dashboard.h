#pragma once

#include <opencv2/opencv.hpp>
#include <string>

class Dashboard {
public:
    Dashboard() = default;

    void draw(
        cv::Mat& frame,
        float speedKmh,
        float rpm,
        float coolantTempC,
        float fuelPercent,
        float throttlePercent,
        const std::string& drivingStyle
    ) const;

private:
    void drawGauge(
        cv::Mat& img,
        const cv::Point& center,
        int radius,
        float value,
        float minValue,
        float maxValue,
        float warningThreshold,
        const std::string& title,
        const std::string& unit,
        const cv::Scalar& normalColor,
        const cv::Scalar& warningColor
    ) const;

    void drawLinearGauge(
        cv::Mat& img,
        const cv::Rect& area,
        float value,
        float minValue,
        float maxValue,
        const std::string& title,
        const std::string& unit,
        const cv::Scalar& fillColor
    ) const;

    static cv::Scalar styleColor(const std::string& drivingStyle);
};
