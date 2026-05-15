#include "obd_parser.h"

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

static fs::path makeTempFile(const std::string& name, const std::string& content) {
    fs::path path = fs::temp_directory_path() / name;
    std::ofstream out(path);
    out << content;
    out.close();
    return path;
}

TEST(OBDParserTest, LabelConversion) {
    EXPECT_EQ(OBDParser::labelToInt("SLOW"), 0);
    EXPECT_EQ(OBDParser::labelToInt("NORMAL"), 1);
    EXPECT_EQ(OBDParser::labelToInt("AGGRESSIVE"), 2);
}

TEST(OBDParserTest, MissingFileReturnsMinusOne) {
    OBDParser parser("this_file_does_not_exist.csv");
    EXPECT_EQ(parser.load(), -1);
}

TEST(OBDParserTest, GetRecordThrowsOnInvalidIndex) {
    const auto path = makeTempFile(
        "obd_getrecord_test.csv",
        "speed_kmh,engine_rpm,throttle_pos,coolant_temp,fuel_level,intake_air_temp,label\n"
        "10,1000,5,80,50,20,SLOW\n"
    );

    OBDParser parser(path.string());
    EXPECT_EQ(parser.load(), 1);

    EXPECT_THROW(parser.getRecord(-1), std::out_of_range);
    EXPECT_THROW(parser.getRecord(1), std::out_of_range);

    fs::remove(path);
}

TEST(OBDParserTest, ParseValidCsv) {
    const auto path = makeTempFile(
        "obd_valid_test.csv",
        "speed_kmh,engine_rpm,throttle_pos,coolant_temp,fuel_level,intake_air_temp,label\n"
        "12.5,900,8,78.0,45.0,19.5,SLOW\n"
        "65.0,2200,35,82.0,40.0,21.0,NORMAL\n"
    );

    OBDParser parser(path.string());
    EXPECT_EQ(parser.load(), 2);
    EXPECT_EQ(parser.size(), 2u);

    const auto& r0 = parser.getRecord(0);
    EXPECT_DOUBLE_EQ(r0.speed_kmh, 12.5);
    EXPECT_DOUBLE_EQ(r0.engine_rpm, 900.0);
    EXPECT_DOUBLE_EQ(r0.throttle_pos, 8.0);
    EXPECT_DOUBLE_EQ(r0.coolant_temp, 78.0);
    EXPECT_DOUBLE_EQ(r0.fuel_level, 45.0);
    EXPECT_DOUBLE_EQ(r0.intake_air_temp, 19.5);
    EXPECT_EQ(r0.label, 0);

    const auto& r1 = parser.getRecord(1);
    EXPECT_EQ(r1.label, 1);

    fs::remove(path);
}

TEST(OBDParserTest, InvalidLineIsSkipped) {
    const auto path = makeTempFile(
        "obd_invalid_line_test.csv",
        "speed_kmh,engine_rpm,throttle_pos,coolant_temp,fuel_level,intake_air_temp,label\n"
        "10,1000,5,80,50,20,SLOW\n"
        "BAD_LINE_SHOULD_BE_SKIPPED\n"
        "70,3000,80,90,30,24,AGGRESSIVE\n"
    );

    OBDParser parser(path.string());
    EXPECT_EQ(parser.load(), 2);
    EXPECT_EQ(parser.size(), 2u);

    EXPECT_EQ(parser.getRecord(0).label, 0);
    EXPECT_EQ(parser.getRecord(1).label, 2);

    fs::remove(path);
}