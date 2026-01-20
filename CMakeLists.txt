cmake_minimum_required(VERSION 3.10)
project(LeCordeur LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Try to find PortAudio using pkg-config
find_package(PkgConfig)
pkg_check_modules(PortAudio REQUIRED portaudio-2.0)

# Main executable
add_executable(LeCordeur
        src/main.cpp
        src/dsp.cpp
        src/config.cpp
        src/ring_buffer.cpp
        src/note_detection.cpp)
target_link_libraries(LeCordeur PRIVATE ${PortAudio_LIBRARIES})
target_include_directories(LeCordeur PRIVATE ${PortAudio_INCLUDE_DIRS}
        LeCordeur PRIVATE include)

# Enable testing
enable_testing()

# Find Google Test
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Test executable
add_executable(LeCordeurTests
        tests/test_frequency_to_note.cpp
        tests/test_dsp.cpp
        src/dsp.cpp
        src/config.cpp
        src/note_detection.cpp)

target_include_directories(LeCordeurTests PRIVATE include)
target_link_libraries(LeCordeurTests PRIVATE GTest::gtest GTest::gtest_main)

# Add tests
add_test(NAME LeCordeurTests COMMAND LeCordeurTests)