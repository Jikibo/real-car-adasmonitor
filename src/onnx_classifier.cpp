#include "onnx_classifier.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cmath>

ONNXClassifier::ONNXClassifier(
    const std::wstring& modelPath,
    const std::string& normalizationJsonPath
)
    : env_(ORT_LOGGING_LEVEL_WARNING, "ADAS"),
      session_(nullptr)
{
    Ort::SessionOptions sessionOptions;
    sessionOptions.SetIntraOpNumThreads(1);
    sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);

    session_ = Ort::Session(env_, modelPath.c_str(), sessionOptions);

    loadNormalizationParams(normalizationJsonPath);

    modelLoaded_ = true;
}

void ONNXClassifier::loadNormalizationParams(const std::string& path) {
    std::ifstream file(path);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open normalization JSON");
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    std::string content = buffer.str();

    mean_ = extractArray(content, "mean");
    std_ = extractArray(content, "std");

    if (mean_.size() != 6 || std_.size() != 6) {
        throw std::runtime_error("Normalization arrays must contain 6 values");
    }
}

std::vector<float> ONNXClassifier::extractArray(
    const std::string& content,
    const std::string& key
) {
    std::vector<float> values;

    std::string searchKey = "\"" + key + "\"";

    auto keyPos = content.find(searchKey);

    if (keyPos == std::string::npos) {
        throw std::runtime_error("Key not found: " + key);
    }

    auto start = content.find('[', keyPos);
    auto end = content.find(']', start);

    if (start == std::string::npos || end == std::string::npos) {
        throw std::runtime_error("Invalid JSON array");
    }

    std::string arrayContent =
        content.substr(start + 1, end - start - 1);

    std::stringstream ss(arrayContent);

    std::string item;

    while (std::getline(ss, item, ',')) {
        values.push_back(std::stof(item));
    }

    return values;
}

std::array<float, 3> ONNXClassifier::softmax(
    const std::vector<float>& logits
) {
    std::array<float, 3> probs{};

    float maxLogit =
        *std::max_element(logits.begin(), logits.end());

    float sum = 0.0f;

    for (float v : logits) {
        sum += std::exp(v - maxLogit);
    }

    for (size_t i = 0; i < 3; ++i) {
        probs[i] = std::exp(logits[i] - maxLogit) / sum;
    }

    return probs;
}

ClassificationResult ONNXClassifier::classify(
    const std::array<float, 6>& features
) {
    if (!modelLoaded_) {
        throw std::runtime_error("Model not loaded");
    }

    std::vector<float> normalized(6);

    for (size_t i = 0; i < 6; ++i) {
        normalized[i] =
            (features[i] - mean_[i]) / std_[i];
    }

    std::array<int64_t, 2> inputShape{1, 6};

    Ort::MemoryInfo memoryInfo =
        Ort::MemoryInfo::CreateCpu(
            OrtArenaAllocator,
            OrtMemTypeDefault
        );

    Ort::Value inputTensor =
        Ort::Value::CreateTensor<float>(
            memoryInfo,
            normalized.data(),
            normalized.size(),
            inputShape.data(),
            inputShape.size()
        );

    const char* inputNames[] = {"features"};
    const char* outputNames[] = {"class_scores"};

    auto outputTensors = session_.Run(
        Ort::RunOptions{nullptr},
        inputNames,
        &inputTensor,
        1,
        outputNames,
        1
    );

    float* rawOutput =
        outputTensors[0].GetTensorMutableData<float>();

    std::vector<float> logits(rawOutput, rawOutput + 3);

    auto probs = softmax(logits);

    int bestLabel = 0;
    float bestScore = probs[0];

    for (int i = 1; i < 3; ++i) {
        if (probs[i] > bestScore) {
            bestScore = probs[i];
            bestLabel = i;
        }
    }

    return {
        bestLabel,
        bestScore,
        probs
    };
}
