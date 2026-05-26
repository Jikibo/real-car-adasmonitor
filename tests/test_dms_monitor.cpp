#include <gtest/gtest.h>
#include "dms_monitor.h"

TEST(DMSMonitorTest, DefaultState) {
    // Проверяет, что модуль не загружен по умолчанию.
}

TEST(DMSMonitorTest, EmptyFrameAnalysis) {
    DMSMonitor monitor("models/deploy.prototxt", "models/res10_300x300_ssd_iter_140000.caffemodel");
    cv::Mat empty_frame(100, 100, CV_8UC3);
    auto result = monitor.analyze(empty_frame);
    ASSERT_FALSE(result.face_detected);
}

TEST(DMSMonitorTest, ModelLoading) {
    DMSMonitor monitor("models/deploy.prototxt", "models/res10_300x300_ssd_iter_140000.caffemodel");
    // Проверить, что модели загружены
}
