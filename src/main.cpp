#include "obd_parser.h"

#include <iostream>
#include <array>

int main() {
    OBDParser parser("../data/dataset_clean_10000.csv");

    const int loaded = parser.load();
    if (loaded < 0) {
        std::cerr << "Failed to load CSV\n";
        return 1;
    }

    std::cout << "Loaded records: " << loaded << "\n\n";

    const int showCount = std::min(5, loaded);
    for (int i = 0; i < showCount; ++i) {
        const auto& r = parser.getRecord(i);
        std::cout << i << ": "
                  << "speed=" << r.speed_kmh
                  << ", rpm=" << r.engine_rpm
                  << ", throttle=" << r.throttle_pos
                  << ", coolant=" << r.coolant_temp
                  << ", fuel=" << r.fuel_level
                  << ", intake=" << r.intake_air_temp
                  << ", label=" << OBDParser::intToLabel(r.label)
                  << "\n";
    }

    std::array<int, 3> counts{0, 0, 0};
    for (std::size_t i = 0; i < parser.size(); ++i) {
        const auto& r = parser.getRecord(static_cast<int>(i));
        if (r.label >= 0 && r.label < 3) {
            counts[r.label]++;
        }
    }

    std::cout << "\nClass statistics:\n";
    std::cout << "SLOW: " << counts[0] << "\n";
    std::cout << "NORMAL: " << counts[1] << "\n";
    std::cout << "AGGRESSIVE: " << counts[2] << "\n";

    return 0;
}