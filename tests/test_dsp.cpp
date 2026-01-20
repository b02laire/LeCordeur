#include <gtest/gtest.h>
#include <cmath>
#include "../include/dsp.hpp"
#include "../include/config.hpp"

class DSPTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Test Hann Window
TEST_F(DSPTest, HannWindowSymmetry) {
    CArray samples(8);
    for (int i = 0; i < 8; i++) {
        samples[i] = {1.0, 0.0};  // Initialize with ones
    }

    applyHannWindow(samples);

    // Hann window should be symmetric
    EXPECT_NEAR(std::abs(samples[0]), std::abs(samples[7]), 1e-10);
    EXPECT_NEAR(std::abs(samples[1]), std::abs(samples[6]), 1e-10);
    EXPECT_NEAR(std::abs(samples[2]), std::abs(samples[5]), 1e-10);
    EXPECT_NEAR(std::abs(samples[3]), std::abs(samples[4]), 1e-10);
}

TEST_F(DSPTest, HannWindowEdges) {
    CArray samples(8);
    for (int i = 0; i < 8; i++) {
        samples[i] = {1.0, 0.0};
    }

    applyHannWindow(samples);

    // Hann window should be zero at edges
    EXPECT_NEAR(std::abs(samples[0]), 0.0, 1e-10);
    EXPECT_NEAR(std::abs(samples[7]), 0.0, 1e-10);
}

TEST_F(DSPTest, HannWindowPeak) {
    CArray samples(8);
    for (int i = 0; i < 8; i++) {
        samples[i] = {1.0, 0.0};
    }

    applyHannWindow(samples);

    // Middle values should be larger than edge values
    EXPECT_GT(std::abs(samples[3]), std::abs(samples[0]));
    EXPECT_GT(std::abs(samples[4]), std::abs(samples[0]));
    EXPECT_GT(std::abs(samples[3]), std::abs(samples[1]));
}

// Test Flat-Top Window
TEST_F(DSPTest, FlatTopWindowSymmetry) {
    CArray samples(8);
    for (int i = 0; i < 8; i++) {
        samples[i] = {1.0, 0.0};
    }

    applyFlatTopWindow(samples);

    // Flat-top window should be symmetric
    EXPECT_NEAR(std::abs(samples[0]), std::abs(samples[7]), 1e-10);
    EXPECT_NEAR(std::abs(samples[1]), std::abs(samples[6]), 1e-10);
    EXPECT_NEAR(std::abs(samples[2]), std::abs(samples[5]), 1e-10);
    EXPECT_NEAR(std::abs(samples[3]), std::abs(samples[4]), 1e-10);
}

// Test FFT with DC component (zero frequency)
TEST_F(DSPTest, FFT_DCComponent) {
    CArray samples(8);
    // Input: constant signal (DC component)
    for (int i = 0; i < 8; i++) {
        samples[i] = {1.0, 0.0};
    }

    fft(samples);

    // All energy should be in DC bin (index 0)
    EXPECT_GT(std::abs(samples[0]), 1.0);

    // Other bins should be approximately zero
    for (int i = 1; i < 8; i++) {
        EXPECT_NEAR(std::abs(samples[i]), 0.0, 1e-10);
    }
}

// Test FFT with single frequency
TEST_F(DSPTest, FFT_SingleFrequency) {
    int N = 32;
    CArray samples(N);

    // Generate a sine wave at bin 4
    double freq = 4.0;
    for (int i = 0; i < N; i++) {
        samples[i] = {std::sin(2.0 * M_PI * freq * i / N), 0.0};
    }

    fft(samples);

    // Peak should be at bin 4 and bin N-4 (mirror)
    double maxMag = 0.0;
    int maxIdx = 0;
    for (int i = 0; i < N/2; i++) {
        if (std::abs(samples[i]) > maxMag) {
            maxMag = std::abs(samples[i]);
            maxIdx = i;
        }
    }

    EXPECT_EQ(maxIdx, 4);
}

// Test FFT Parseval's theorem (energy conservation)
TEST_F(DSPTest, FFT_EnergyConservation) {
    int N = 16;
    CArray samples(N);

    // Generate random signal and compute energy
    double energyTime = 0.0;
    for (int i = 0; i < N; i++) {
        samples[i] = {static_cast<double>(i % 3), 0.0};
        energyTime += std::abs(samples[i]) * std::abs(samples[i]);
    }

    fft(samples);

    // Compute energy in frequency domain
    double energyFreq = 0.0;
    for (int i = 0; i < N; i++) {
        energyFreq += std::abs(samples[i]) * std::abs(samples[i]);
    }

    // Energy should be conserved (scaled by N)
    EXPECT_NEAR(energyTime, energyFreq / N, 1e-6);
}

// Test FFT size requirements
TEST_F(DSPTest, FFT_PowerOfTwo) {
    // FFT should work with power-of-two sizes
    std::vector<int> sizes = {2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};

    for (int N : sizes) {
        CArray samples(N);
        for (int i = 0; i < N; i++) {
            samples[i] = {1.0, 0.0};
        }

        // Should not crash
        EXPECT_NO_THROW(fft(samples));
    }
}

// Test FFT with complex input
TEST_F(DSPTest, FFT_ComplexInput) {
    int N = 8;
    CArray samples(N);

    for (int i = 0; i < N; i++) {
        samples[i] = {std::cos(2.0 * M_PI * i / N), std::sin(2.0 * M_PI * i / N)};
    }

    fft(samples);

    // Should complete without error and produce non-zero output
    double totalMag = 0.0;
    for (int i = 0; i < N; i++) {
        totalMag += std::abs(samples[i]);
    }

    EXPECT_GT(totalMag, 0.0);
}

// Test window functions preserve array size
TEST_F(DSPTest, WindowFunctionsPreserveSize) {
    CArray samples(16);
    for (int i = 0; i < 16; i++) {
        samples[i] = {1.0, 0.0};
    }

    applyHannWindow(samples);
    EXPECT_EQ(samples.size(), 16);

    CArray samples2(16);
    for (int i = 0; i < 16; i++) {
        samples2[i] = {1.0, 0.0};
    }

    applyFlatTopWindow(samples2);
    EXPECT_EQ(samples2.size(), 16);
}
