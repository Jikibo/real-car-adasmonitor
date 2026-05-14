cmake_minimum_required(VERSION 3.16)

project(RealCarADASMonitor)

# =========================
# C++ Standard
# =========================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(OpenCV_DIR "C:/CommonApps/opencv/build")

# =========================
# OpenCV
# =========================
find_package(OpenCV REQUIRED)

# =========================
# ONNX Runtime
# =========================
set(ONNXRUNTIME_DIR "C:/CommonApps/onnxruntime")

# подключает .h файлы
include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${ONNXRUNTIME_DIR}/include
    src
)

# подключает .lib
link_directories(
    ${ONNXRUNTIME_DIR}/lib
)

# =========================
# Main executable
# Создаёт .exe
# =========================
add_executable(RealCarMonitor
    src/main.cpp
    src/obd_parser.cpp
    src/onnx_classifier.cpp
    src/dashboard.cpp
    src/dms_monitor.cpp
    src/dms_hud.cpp
)

# Подключает OpenCV и ONNX Runtime
target_link_libraries(RealCarMonitor
    ${OpenCV_LIBS}
    onnxruntime
)

# =========================
# Google Test
# Позволяет скачать GoogleTest автоматически.
# =========================
include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/heads/main.zip
	DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(googletest)

enable_testing()

# =========================
# Tests
# =========================
add_executable(RunTests
    tests/test_obd_parser.cpp
    src/obd_parser.cpp
)

target_link_libraries(RunTests
    gtest
    gtest_main
)

add_test(
    NAME RunTests
    COMMAND RunTests
)
