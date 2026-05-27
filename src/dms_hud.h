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
    /**
     * @brief Рисует угловую рамку.
     * @param img Изображение для рисования.
     * @param rect Прямоугольник с координатами.
     * @param color Цвет линии.
     * @param thickness Толщина линии.
     */
    void drawCornerRect(cv::Mat& img, const cv::Rect& rect, const cv::Scalar& color, int thickness = 2) const;

    /**
     * @brief Рисует точку статуса.
     * @param img Изображение для рисования.
     * @param center Центр точки.
     * @param color Цвет точки.
     * @param label Текст метки.
     */
    void drawStatusDot(cv::Mat& img, const cv::Point& center, const cv::Scalar& color, const std::string& label) const;

    /**
     * @brief Рисует центральную полосу с текстом.
     * @param img Изображение для рисования.
     * @param text Текст для отображения.
     * @param color Цвет текста.
     */
    void drawCenteredBanner(cv::Mat& img, const std::string& text, const cv::Scalar& color) const;
};
