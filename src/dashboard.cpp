#include "dashboard.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace {
constexpr int kPanelWidth = 320;   // левая половина кадра 640x480
constexpr int kFrameHeight = 480;

float clampf(float v, float lo, float hi) {
    return std::max(lo, std::min(v, hi));
}

std::string formatValue(float value, const std::string& unit) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(0) << value << unit;
    return oss.str();
}

float valueToAngleDeg(float value, float minValue, float maxValue) {
    // Дуга от 210° до 330°
    constexpr float startAngle = 210.0f;
    constexpr float sweep = 120.0f;

    float t = (value - minValue) / (maxValue - minValue);
    t = clampf(t, 0.0f, 1.0f);
    return startAngle + sweep * t;
}

cv::Point angleToPoint(const cv::Point& center, int radius, float angleDeg) {
    float rad = angleDeg * static_cast<float>(CV_PI) / 180.0f;
    return {
        static_cast<int>(std::lround(center.x + std::cos(rad) * radius)),
        static_cast<int>(std::lround(center.y + std::sin(rad) * radius))
    };
}
} // namespace

cv::Scalar Dashboard::styleColor(const std::string& drivingStyle) {
    if (drivingStyle == "AGGRESSIVE") return cv::Scalar(0, 0, 255);       // красный
    if (drivingStyle == "SLOW")       return cv::Scalar(255, 255, 0);     // голубой
    return cv::Scalar(0, 255, 0);                                         // NORMAL = зелёный
}

void Dashboard::drawGauge(
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
) const {
    const float clampedValue = clampf(value, minValue, maxValue);
    const float threshold = clampf(warningThreshold, minValue, maxValue);

    const float startAngle = valueToAngleDeg(minValue, minValue, maxValue);
    const float thresholdAngle = valueToAngleDeg(threshold, minValue, maxValue);
    const float endAngle = valueToAngleDeg(maxValue, minValue, maxValue);

    // Внешний контур
    cv::circle(img, center, radius + 8, cv::Scalar(90, 90, 90), 1, cv::LINE_AA);
    cv::circle(img, center, radius, cv::Scalar(50, 50, 50), 2, cv::LINE_AA);

    // Цветные дуги
    cv::ellipse(img, center, cv::Size(radius, radius), 0.0, startAngle, thresholdAngle,
                normalColor, 8, cv::LINE_AA);
    cv::ellipse(img, center, cv::Size(radius, radius), 0.0, thresholdAngle, endAngle,
                warningColor, 8, cv::LINE_AA);

    // Деления
    const int ticks = 7;
    for (int i = 0; i <= ticks; ++i) {
        float t = static_cast<float>(i) / ticks;
        float a = startAngle + (endAngle - startAngle) * t;
        cv::Point p1 = angleToPoint(center, radius - 2, a);
        cv::Point p2 = angleToPoint(center, radius - 12, a);
        cv::line(img, p1, p2, cv::Scalar(230, 230, 230), 1, cv::LINE_AA);
    }

    // Стрелка
    float needleAngle = valueToAngleDeg(clampedValue, minValue, maxValue);
    cv::Point needleTip = angleToPoint(center, radius - 16, needleAngle);
    cv::line(img, center, needleTip, cv::Scalar(240, 240, 240), 2, cv::LINE_AA);
    cv::circle(img, center, 4, cv::Scalar(240, 240, 240), cv::FILLED, cv::LINE_AA);

    // Заголовок и значение
    cv::putText(img, title, {center.x - radius, center.y - radius - 12},
                cv::FONT_HERSHEY_SIMPLEX, 0.55, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);

    std::string valueText = formatValue(clampedValue, unit);
    cv::putText(img, valueText, {center.x - 34, center.y + radius + 20},
                cv::FONT_HERSHEY_SIMPLEX, 0.55, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
}

void Dashboard::drawLinearGauge(
    cv::Mat& img,
    const cv::Rect& area,
    float value,
    float minValue,
    float maxValue,
    const std::string& title,
    const std::string& unit,
    const cv::Scalar& fillColor
) const {
    const float clampedValue = clampf(value, minValue, maxValue);
    const float t = (clampedValue - minValue) / (maxValue - minValue);

    // Подпись
    cv::putText(img, title, {area.x, area.y - 8},
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);

    // Фон полосы
    cv::rectangle(img, area, cv::Scalar(55, 55, 55), cv::FILLED, cv::LINE_AA);
    cv::rectangle(img, area, cv::Scalar(110, 110, 110), 1, cv::LINE_AA);

    // Заполнение
    int fillWidth = static_cast<int>(std::lround(area.width * t));
    fillWidth = std::clamp(fillWidth, 0, area.width);

    if (fillWidth > 0) {
        cv::Rect fillRect(area.x, area.y, fillWidth, area.height);
        cv::rectangle(img, fillRect, fillColor, cv::FILLED, cv::LINE_AA);
    }

    // Значение справа
    std::string valueText = formatValue(clampedValue, unit);
    cv::putText(img, valueText, {area.x + area.width + 10, area.y + area.height - 2},
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
}

void Dashboard::draw(
    cv::Mat& frame,
    float speedKmh,
    float rpm,
    float coolantTempC,
    float fuelPercent,
    float throttlePercent,
    const std::string& drivingStyle
) const {
    if (frame.empty()) {
        return;
    }

    if (frame.cols < 640 || frame.rows < 480) {
        throw std::runtime_error("Dashboard expects at least 640x480 frame");
    }

    cv::Mat overlay = frame.clone();
    cv::Rect leftPanel(0, 0, frame.cols / 2, frame.rows);

    // Фон панели
    cv::rectangle(overlay, leftPanel, cv::Scalar(25, 25, 25), cv::FILLED, cv::LINE_AA);
    cv::rectangle(overlay, leftPanel, cv::Scalar(70, 70, 70), 1, cv::LINE_AA);

    // Заголовок
    cv::putText(overlay, "DASHBOARD", {14, 28},
                cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);

    // Два круговых прибора
    drawGauge(
        overlay,
        cv::Point(85, 145),
        58,
        speedKmh,
        0.0f,
        140.0f,
        90.0f,
        "SPEED",
        " km/h",
        cv::Scalar(0, 220, 0),
        cv::Scalar(0, 0, 255)
    );

    drawGauge(
        overlay,
        cv::Point(235, 145),
        58,
        rpm,
        0.0f,
        6000.0f,
        4500.0f,
        "RPM",
        "",
        cv::Scalar(0, 220, 0),
        cv::Scalar(0, 0, 255)
    );

    // Горизонтальные полосы
    drawLinearGauge(
        overlay,
        cv::Rect(20, 250, 210, 18),
        coolantTempC,
        0.0f,
        120.0f,
        "COOLANT",
        " C",
        cv::Scalar(0, 180, 255)
    );

    drawLinearGauge(
        overlay,
        cv::Rect(20, 305, 210, 18),
        fuelPercent,
        0.0f,
        100.0f,
        "FUEL",
        " %",
        cv::Scalar(0, 200, 0)
    );

    drawLinearGauge(
        overlay,
        cv::Rect(20, 360, 210, 18),
        throttlePercent,
        0.0f,
        100.0f,
        "THROTTLE",
        " %",
        cv::Scalar(255, 180, 0)
    );

    // Стиль вождения
    cv::Scalar styleCol = styleColor(drivingStyle);
    cv::putText(overlay, "STYLE:", {20, 410},
                cv::FONT_HERSHEY_SIMPLEX, 0.55, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
    cv::putText(overlay, drivingStyle, {85, 410},
                cv::FONT_HERSHEY_SIMPLEX, 0.62, styleCol, 2, cv::LINE_AA);

    // Предупреждения
    bool tempWarn = coolantTempC > 100.0f;
    bool fuelWarn = fuelPercent < 15.0f;

    if (tempWarn || fuelWarn) {
        cv::Rect warnBox(12, 430, leftPanel.width - 24, 38);
        cv::rectangle(overlay, warnBox, cv::Scalar(0, 140, 255), cv::FILLED, cv::LINE_AA);
        cv::rectangle(overlay, warnBox, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);

        std::string warnText;
        if (tempWarn && fuelWarn) {
            warnText = "WARNING: OVERHEAT + LOW FUEL";
        } else if (tempWarn) {
            warnText = "WARNING: OVERHEAT";
        } else {
            warnText = "WARNING: LOW FUEL";
        }

        cv::putText(overlay, warnText, {18, 455},
                    cv::FONT_HERSHEY_SIMPLEX, 0.48, cv::Scalar(0, 0, 0), 2, cv::LINE_AA);
    }

    // Прозрачное наложение только на левую половину кадра
    cv::addWeighted(
        overlay(leftPanel), 0.90,
        frame(leftPanel),   0.10,
        0.0,
        frame(leftPanel)
    );
}
