#include "dms_monitor.h"
#include "dms_hud.h"

#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Cannot open camera\n";
        return 1;
    }

    DMSMonitor monitor("../models");
    if (!monitor.isReady()) {
        std::cerr << "DMS models are not loaded. Check ../models/\n";
        return 1;
    }

    DMSHUD hud;

    cv::Mat frame;
    while (true) {
        cap >> frame;
        if (frame.empty()) {
            break;
        }

        DriverState state = monitor.analyze(frame);
        cv::Mat view = hud.draw(frame, state);

        if (view.empty()) {
            std::cerr << "HUD render failed\n";
            break;
        }

        cv::imshow("DMS", view);

        int key = cv::waitKey(1);
        if (key == 27 || key == 'q' || key == 'Q') {
            break;
        }
    }

    return 0;
}
