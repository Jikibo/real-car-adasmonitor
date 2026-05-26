#include "onnx_classifier.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>

ONNXClassifier::ONNXClassifier(const std::string& modelPath, const std::string& paramsPath) {
    readNormalizationParams(paramsPath);
}

void ONNXClassifier::readNormalizationParams(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open normalization_params.json");
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    // Извлечение mean
    std::regex meanRegex("\"mean\"\\s*:\\s*\\[([^]]+)\\]");
    std::smatch match;
    if (std::regex_search(content, match, meanRegex)) {
        std::string numbers = match[1].str();
        std::stringstream ss(numbers);
        float num;
        while (ss >> num) {
            mean_.push_back(num);
            ss.ignore(1); // пропуск запятой
        }
    }

    // Извлечение std
    std::regex stdRegex("\"std\"\\s*:\\s*\\[([^]]+)\\]");
    if (std::regex_search(content, match, stdRegex)) {
        std::string numbers = match[1].str();
        std::stringstream ss(numbers);
        float num;
        while (ss >> num) {
            std_.push_back(num);
            ss.ignore(1);
        }
    }
}

ClassificationResult ONNXClassifier::classify(const OBDRecord& record) {
    ClassificationResult result{};
    result.label = record.label;   // временно, только чтобы проверить отображение
    result.confidence = 1.0f;
    result.scores = {0.0f, 0.0f, 0.0f};
    result.scores[record.label] = 1.0f;
    return result;
}
