#pragma once

#include <onnxruntime_cxx_api.h>


struct ClassificationResult {
    int label;
    float confidence;
    std::array<float, 3> scores;
};

class ONNXClassifier {
public:
    ONNXClassifier(
        const std::wstring& modelPath,
        const std::string& normalizationJsonPath
    );

    ClassificationResult classify(const std::array<float, 6>& features);

private:
    void loadNormalizationParams(const std::string& path);

    static std::vector<float> extractArray(
        const std::string& content,
        const std::string& key
    );

    static std::array<float, 3> softmax(const std::vector<float>& logits);

private:
    Ort::Env env_;
    Ort::Session session_;

    std::vector<float> mean_;
    std::vector<float> std_;

    bool modelLoaded_{false};
};
