#include "dms_hud.h"

#include <algorithm>
#include <string>

cv::Mat DMSHUD::draw(const cv::Mat& camera_frame, const DriverState& state) const {
    if (camera_frame.empty()) {
        return cv::Mat();
    }

    cv::Mat canvas(camera_frame.rows, camera_frame.cols * 2, CV_8UC3, cv::Scalar(18, 18, 18));

    cv::Rect left_panel(0, 0, camera_frame.cols, camera_frame.rows);
    cv::Rect right_panel(camera_frame.cols, 0, camera_frame.cols, camera_frame.rows);

    cv::Mat left_roi = canvas(left_panel);
    cv::Mat right_roi = canvas(right_panel);

    if (right_roi.size() == camera_frame.size()) {
        camera_frame.copyTo(right_roi);
    } else {
        cv::resize(camera_frame, right_roi, right_roi.size());
    }

    cv::putText(
        left_roi,
        "DMS STATUS",
        cv::Point(24, 42),
        cv::FONT_HERSHEY_SIMPLEX,
        1.0,
        cv::Scalar(240, 240, 240),
        2
    );

    std::string driverStatus = "NORMAL";
    cv::Scalar statusColor(0, 255, 0);

    if (state.alert_drowsy) {
        driverStatus = "FATIGUE DETECTED";
        statusColor = cv::Scalar(0, 165, 255);
    } else if (state.alert_distracted) {
        driverStatus = "DISTRACTED";
        statusColor = cv::Scalar(0, 0, 255);
    }

    cv::rectangle(
        left_roi,
        cv::Rect(20, 58, left_roi.cols - 40, 46),
        statusColor,
        cv::FILLED,
        cv::LINE_AA
    );

    cv::putText(
        left_roi,
        driverStatus,
        cv::Point(36, 90),
        cv::FONT_HERSHEY_SIMPLEX,
        0.95,
        cv::Scalar(255, 255, 255),
        2,
        cv::LINE_AA
    );

    const int y0 = 145;
    const int dy = 58;

    drawStatusDot(
        left_roi,
        cv::Point(34, y0),
        state.face_detected ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255),
        state.face_detected ? "Face detected" : "Face not found"
    );

    drawStatusDot(
        left_roi,
        cv::Point(34, y0 + dy),
        state.eyes_open ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 165, 255),
        state.eyes_open ? "Eyes open" : "Eyes closed"
    );

    drawStatusDot(
        left_roi,
        cv::Point(34, y0 + dy * 2),
        state.looking_forward ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255),
        state.looking_forward ? "Looking forward" : "Distracted"
    );

    drawStatusDot(
        left_roi,
        cv::Point(34, y0 + dy * 3),
        state.alert_drowsy ? cv::Scalar(0, 165, 255) : cv::Scalar(0, 255, 0),
        state.alert_drowsy ? "Drowsiness alert" : "No drowsiness"
    );

    drawStatusDot(
        left_roi,
        cv::Point(34, y0 + dy * 4),
        state.alert_distracted ? cv::Scalar(0, 0, 255) : cv::Scalar(0, 255, 0),
        state.alert_distracted ? "Distraction alert" : "No distraction"
    );

    char buf[128];
    std::snprintf(buf, sizeof(buf), "Eye openness: %.2f", state.eye_openness);
    cv::putText(
        left_roi,
        buf,
        cv::Point(24, left_roi.rows - 78),
        cv::FONT_HERSHEY_SIMPLEX,
        0.75,
        cv::Scalar(230, 230, 230),
        2
    );

    std::snprintf(buf, sizeof(buf), "Head turn: %.1f deg", state.head_turn_deg);
    cv::putText(
        left_roi,
        buf,
        cv::Point(24, left_roi.rows - 38),
        cv::FONT_HERSHEY_SIMPLEX,
        0.75,
        cv::Scalar(230, 230, 230),
        2
    );

    if (state.face_detected) {
        cv::Rect face_on_canvas = state.face_rect + cv::Point(camera_frame.cols, 0);
        cv::Scalar frame_color = state.alert_drowsy ? cv::Scalar(0, 165, 255) : cv::Scalar(0, 255, 0);
        drawCornerRect(canvas, face_on_canvas, frame_color, 3);
    }

    if (state.alert_drowsy) {
        cv::rectangle(
            canvas,
            cv::Rect(6, 6, canvas.cols - 12, canvas.rows - 12),
            cv::Scalar(0, 165, 255),
            4
        );
        drawCenteredBanner(canvas, "DROWSINESS ALERT", cv::Scalar(0, 165, 255));
    }

    if (state.alert_distracted) {
        cv::rectangle(
            canvas,
            cv::Rect(0, canvas.rows - 46, canvas.cols, 46),
            cv::Scalar(0, 0, 255),
            cv::FILLED
        );
        cv::putText(
            canvas,
            "DISTRACTION ALERT",
            cv::Point(24, canvas.rows - 14),
            cv::FONT_HERSHEY_SIMPLEX,
            0.8,
            cv::Scalar(255, 255, 255),
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
        2
    );
}

void DMSHUD::drawCenteredBanner(cv::Mat& img, const std::string& text, const cv::Scalar& color) const {
    int baseline = 0;
    const double scale = 1.0;
    const int thickness = 2;

    cv::Size text_size = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, scale, thickness, &baseline);

    const int pad_x = 24;
    const int pad_y = 16;

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
