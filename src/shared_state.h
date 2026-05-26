#ifndef SHARED_STATE_H
#define SHARED_STATE_H

#include <mutex>
#include <atomic>
#include "obd_parser.h"
#include "onnx_classifier.h"

struct SharedState {
    OBDRecord current_record;
    ClassificationResult classification_result;
    int alert_count = 0;
    std::atomic<bool> running{true};
    std::mutex mtx;

    // Конструктор
    SharedState() = default;
};

#endif // SHARED_STATE_H
