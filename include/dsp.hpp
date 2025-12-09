#pragma once
#include "config.hpp"

void applyHannWindow(CArray& samples);
void applyFlatTopWindow(CArray& samples);
void fft(CArray& samples);