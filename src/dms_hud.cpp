#include "dms_hud.h"

#include <algorithm>
#include <cstdio>
#include <string>

namespace {
cv::Rect clampRect(const cv::Rect& r, const cv::Size& size) {
    return r & cv::Rect(0, 0, size.width, size.height);
}

cv::Rect scaleRectToTarget(const cv::Rect& src, const cv::Size& srcSize, const cv::Size& dstSize) {
    if (src.area() <= 0 || srcSize.width <= 0 || srcSize.height <= 0 ||
        dstSize.width <= 0 || dstSize.height <= 0) {
        return {};
    }

    const float sx = static_cast<float>(dstSize.width) / static_cast<float>(srcSize.width);
    const float sy = static_cast<float>(dstSize.height) / static_cast<float>(srcSize.height);

    cv::Rect scaled(
        static_cast<int>(std::lround(src.x * sx)),
        static_cast<int>(std::lround(src.y * sy)),
        static_cast<int>(std::lround(src.width * sx)),
        static_cast<int>(std::lround(src.height * sy))
    );

    return clampRect(scaled, dstSize);
}
} // namespace

cv::Mat DMSHUD::draw(const cv::Mat& camera_frame, const DriverState& state) const {
    if (camera_frame.empty()) {
        return cv::Mat();
    }

    cv::Mat canvas = camera_frame.clone();

    cv::putText(
        canvas,
        "DMS STATUS",
        cv::Point(20, 35),
        cv::FONT_HERSHEY_SIMPLEX,
        0.9,
        cv::Scalar(255, 255, 255),
        2
    );

    const int y0 = 75;
    const int dy = 42;

    drawStatusDot(
        canvas,
        cv::Point(25, y0),
        state.face_detected ? cv::Scalar(0,255,0) : cv::Scalar(0,0,255),
        state.face_detected ? "Face detected" : "Face not found"
    );

    drawStatusDot(
        canvas,
        cv::Point(25, y0 + dy),
        state.eyes_open ? cv::Scalar(0,255,0) : cv::Scalar(0,165,255),
        state.eyes_open ? "Eyes open" : "Eyes closed"
    );

    drawStatusDot(
        canvas,
        cv::Point(25, y0 + dy * 2),
        state.looking_forward ? cv::Scalar(0,255,0) : cv::Scalar(0,0,255),
        state.looking_forward ? "Looking forward" : "Distracted"
    );

    drawStatusDot(
        canvas,
        cv::Point(25, y0 + dy * 3),
        state.alert_drowsy ? cv::Scalar(0,165,255) : cv::Scalar(0,255,0),
        state.alert_drowsy ? "Drowsiness alert" : "No drowsiness"
    );

    if (state.face_detected) {
        cv::Scalar face_color =
            state.alert_drowsy
                ? cv::Scalar(0,165,255)
                : cv::Scalar(0,255,0);

        drawCornerRect(canvas, state.face_rect, face_color, 3);
    }

    if (state.alert_drowsy) {
        drawCenteredBanner(
            canvas,
            "DROWSINESS ALERT",
            cv::Scalar(0,165,255)
        );
    }

    if (state.alert_distracted) {
        cv::rectangle(
            canvas,
            cv::Rect(0, canvas.rows - 50, canvas.cols, 50),
            cv::Scalar(0,0,255),
            cv::FILLED
        );

        cv::putText(
            canvas,
            "DISTRACTION ALERT",
            cv::Point(20, canvas.rows - 15),
            cv::FONT_HERSHEY_SIMPLEX,
            0.8,
            cv::Scalar(255,255,255),
            2
        );
    }

    return canvas;
}
void DMSHUD::drawCornerRect(cv::Mat& img, const cv::Rect& rect, const cv::Scalar& color, int thickness) const {
    if (rect.area() <= 0) {
        return;
    }

    const int l = std::max(18, std::min(rect.width, rect.height) / 4);

    const cv::Point tl(rect.x, rect.y);
    const cv::Point tr(rect.x + rect.width, rect.y);
    const cv::Point bl(rect.x, rect.y + rect.height);
    const cv::Point br(rect.x + rect.width, rect.y + rect.height);

    auto line = [&](const cv::Point& a, const cv::Point& b) {
        cv::line(img, a, b, color, thickness, cv::LINE_AA);
    };

    line(tl, cv::Point(tl.x + l, tl.y));
    line(tl, cv::Point(tl.x, tl.y + l));

    line(tr, cv::Point(tr.x - l, tr.y));
    line(tr, cv::Point(tr.x, tr.y + l));

    line(bl, cv::Point(bl.x + l, bl.y));
    line(bl, cv::Point(bl.x, bl.y - l));

    line(br, cv::Point(br.x - l, br.y));
    line(br, cv::Point(br.x, br.y - l));
}

void DMSHUD::drawStatusDot(cv::Mat& img, const cv::Point& center, const cv::Scalar& color, const std::string& label) const {
    cv::circle(img, center, 8, color, cv::FILLED, cv::LINE_AA);
    cv::putText(
        img,
        label,
        cv::Point(center.x + 22, center.y + 6),
        cv::FONT_HERSHEY_SIMPLEX,
        0.68,
        cv::Scalar(235, 235, 235),
        2,
        cv::LINE_AA
    );
}

void DMSHUD::drawCenteredBanner(cv::Mat& img, const std::string& text, const cv::Scalar& color) const {
    int baseline = 0;
    const double scale = 1.0;
    const int thickness = 2;

    cv::Size text_size = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, scale, thickness, &baseline);

    const int pad_x = 24;
    const int pad_y = 14;

    cv::Rect box(
        (img.cols - text_size.width) / 2 - pad_x,
        (img.rows - text_size.height) / 2 - pad_y,
        text_size.width + pad_x * 2,
        text_size.height + pad_y * 2
    );

    box &= cv::Rect(0, 0, img.cols, img.rows);

    cv::rectangle(img, box, color, cv::FILLED, cv::LINE_AA);

    cv::putText(
        img,
        text,
        cv::Point(box.x + pad_x, box.y + box.height / 2 + text_size.height / 2 - 4),
        cv::FONT_HERSHEY_SIMPLEX,
        scale,
        cv::Scalar(255, 255, 255),
        thickness,
        cv::LINE_AA
    );
}
