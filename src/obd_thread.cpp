#include <thread>
#include <chrono>
#include "shared_state.h"
#include "obd_parser.h"
#include "onnx_classifier.h"

void obd_thread_function(OBDParser& parser, ONNXClassifier& classifier, SharedState& state) {
    int index = 0;
    while (state.running.load()) {
        auto record = parser.getRecord(index++);
        if (!record.has_value()) {
            break; // Достигнут конец файла
        }

        auto result = classifier.classify(record.value());
        
        {
            std::lock_guard<std::mutex> lock(state.mtx);
            state.current_record = record.value();
            state.classification_result = result;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 10 Hz
    }
}
