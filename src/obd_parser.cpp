#include "obd_parser.h"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

OBDParser::OBDParser(std::string filename)
    : filename_(std::move(filename)) {}

std::size_t OBDParser::size() const noexcept {
    return records_.size();
}

int OBDParser::labelToInt(const std::string& label) {
    if (label == "SLOW") return 0;
    if (label == "NORMAL") return 1;
    if (label == "AGGRESSIVE") return 2;
    throw std::invalid_argument("Unknown label: " + label);
}

std::string OBDParser::intToLabel(int label) {
    switch (label) {
        case 0: return "SLOW";
        case 1: return "NORMAL";
        case 2: return "AGGRESSIVE";
        default: throw std::invalid_argument("Unknown label id");
    }
}

std::string OBDParser::trim(const std::string& s) {
    const auto begin = std::find_if_not(s.begin(), s.end(),
        [](unsigned char ch) { return std::isspace(ch); });

    const auto end = std::find_if_not(s.rbegin(), s.rend(),
        [](unsigned char ch) { return std::isspace(ch); }).base();

    if (begin >= end) return "";
    return std::string(begin, end);
}

bool OBDParser::parseLine(const std::string& line, OBDRecord& record, std::string& error) const {
    std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string item;

    while (std::getline(ss, item, ',')) {
        fields.push_back(trim(item));
    }

    if (fields.size() != 7) {
        error = "expected 7 columns, got " + std::to_string(fields.size());
        return false;
    }

    try {
        record.speed_kmh = std::stod(fields[0]);
        record.engine_rpm = std::stod(fields[1]);
        record.throttle_pos = std::stod(fields[2]);
        record.coolant_temp = std::stod(fields[3]);
        record.fuel_level = std::stod(fields[4]);
        record.intake_air_temp = std::stod(fields[5]);
        record.label = labelToInt(fields[6]);
    } catch (const std::exception& e) {
        error = e.what();
        return false;
    }

    return true;
}

int OBDParser::load() {
    records_.clear();

    std::ifstream file(filename_);
    if (!file.is_open()) {
        std::cerr << "OBDParser: file not found: " << filename_ << '\n';
        return -1;
    }

    std::string line;
    if (!std::getline(file, line)) {
        std::cerr << "OBDParser: empty file: " << filename_ << '\n';
        return -1;
    }

    std::size_t lineNo = 2; // because header was read already

    while (std::getline(file, line)) {
        if (trim(line).empty()) {
            ++lineNo;
            continue;
        }

        OBDRecord record{};
        std::string error;
        if (!parseLine(line, record, error)) {
            std::cerr << "OBDParser warning: skip line " << lineNo
                      << " (" << error << ")\n";
            ++lineNo;
            continue;
        }

        records_.push_back(record);
        ++lineNo;
    }

    return static_cast<int>(records_.size());
}

const OBDRecord& OBDParser::getRecord(int index) const {
    if (index < 0 || static_cast<std::size_t>(index) >= records_.size()) {
        throw std::out_of_range("OBDParser::getRecord index out of range");
    }
    return records_[static_cast<std::size_t>(index)];
}