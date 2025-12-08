cmake_minimum_required(VERSION 3.10)
project(LeCordeur LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Try to find PortAudio using pkg-config
find_package(PkgConfig)
pkg_check_modules(PortAudio REQUIRED portaudio-2.0)

add_executable(LeCordeur src/main.cpp)
target_link_libraries(LeCordeur PRIVATE ${PortAudio_LIBRARIES})
target_include_directories(LeCordeur PRIVATE ${PortAudio_INCLUDE_DIRS})