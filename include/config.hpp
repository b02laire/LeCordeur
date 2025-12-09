#pragma once
#include <complex>
#include <vector>
#include <atomic>

#define SAMPLE_RATE 48000
#define FRAMES_PER_BUFFER 4096

using Complex = std::complex<double>;
using CArray = std::vector<Complex>;

extern std::atomic<bool> isRecording;