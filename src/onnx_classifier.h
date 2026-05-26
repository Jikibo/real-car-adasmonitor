#ifndef ONNX_CLASSIFIER_H
#define ONNX_CLASSIFIER_H

#include <string>
#include <vector>
#include "obd_parser.h"

struct ClassificationResult {
    int label;         // 0=SLOW, 1=NORMAL, 2=AGGRESSIVE
    float confidence;
    std::vector<float> scores;
};

class ONNXClassifier {
public:
    explicit ONNXClassifier(const std::string& modelPath, const std::string& paramsPath);
    ClassificationResult classify(const OBDRecord& record);

private:
    std::vector<float> mean_;
    std::vector<float> std_;

    void readNormalizationParams(const std::string& path);
};

#endif
