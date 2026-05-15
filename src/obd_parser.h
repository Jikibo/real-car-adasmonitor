#pragma once

#include <cstddef>
#include <string>
#include <vector>

struct OBDRecord {
    double speed_kmh{};
    double engine_rpm{};
    double throttle_pos{};
    double coolant_temp{};
    double fuel_level{};
    double intake_air_temp{};
    int label{}; // 0 = SLOW, 1 = NORMAL, 2 = AGGRESSIVE
};

class OBDParser {
public:
    explicit OBDParser(std::string filename);

    int load();
    std::size_t size() const noexcept;

    const OBDRecord& getRecord(int index) const;

    static int labelToInt(const std::string& label);
    static std::string intToLabel(int label);

private:
    std::string filename_;
    std::vector<OBDRecord> records_;

    static std::string trim(const std::string& s);
    bool parseLine(const std::string& line, OBDRecord& record, std::string& error) const;
};