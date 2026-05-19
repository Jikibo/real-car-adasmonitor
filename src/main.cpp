#include "obd_parser.h"
#include "onnx_classifier.h"

#include <iostream>
#include <iomanip>

int main() {
    try {
        OBDParser parser("../data/dataset_clean_10000.csv");

        int loaded = parser.load();

        if (loaded <= 0) {
            std::cerr << "Failed to load CSV\n";
            return 1;
        }

        ONNXClassifier classifier(
            L"../models/driver_classifier.onnx",
            "../models/normalization_params.json"
        );

        int correct = 0;
        int total = std::min(20, static_cast<int>(parser.size()));

        std::cout
            << "TRUE\tPRED\tCONFIDENCE\n";

        for (int i = 0; i < total; ++i) {
            const auto& r = parser.getRecord(i);

            std::array<float, 6> features = {
                static_cast<float>(r.speed_kmh),
                static_cast<float>(r.engine_rpm),
                static_cast<float>(r.throttle_pos),
                static_cast<float>(r.coolant_temp),
                static_cast<float>(r.fuel_level),
                static_cast<float>(r.intake_air_temp)
            };

            auto result = classifier.classify(features);

            std::cout
                << OBDParser::intToLabel(r.label)
                << "\t"
                << OBDParser::intToLabel(result.label)
                << "\t"
                << std::fixed
                << std::setprecision(3)
                << result.confidence
                << "\n";

            if (r.label == result.label) {
                ++correct;
            }
        }

        float accuracy =
            static_cast<float>(correct) / total;

        std::cout << "\nAccuracy: "
                  << accuracy * 100.0f
                  << "%\n";
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    return 0;
}
