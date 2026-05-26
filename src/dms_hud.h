#pragma once

#include "dms_monitor.h"

#include <opencv2/opencv.hpp>
#include <string>

/**
 * @brief HUD для отображения состояния водителя и камеры.
 *
 * Левую часть панели занимает список статусов,
 * правую — изображение с камеры с рамкой лица.
 */
class DMSHUD {
public:
    /**
     * @brief Рисует DMS-панель.
     * @param camera_frame Кадр с камеры.
     * @param state Состояние водителя.
     * @return Готовый кадр панели DMS.
     */
    cv::Mat draw(const cv::Mat& camera_frame, const DriverState& state) const;

private:
    void drawCornerRect(cv::Mat& img, const cv::Rect& rect, const cv::Scalar& color, int thickness = 2) const;
    void drawStatusDot(cv::Mat& img, const cv::Point& center, const cv::Scalar& color, const std::string& label) const;
    void drawCenteredBanner(cv::Mat& img, const std::string& text, const cv::Scalar& color) const;
};