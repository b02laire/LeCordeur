#include <gtest/gtest.h>
#include <cmath>
#include <string>
#include "../include/note_detection.hpp"

class FrequencyToNoteTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Test exact standard note frequencies
TEST_F(FrequencyToNoteTest, StandardNoteFrequencies) {
    EXPECT_EQ(frequencyToNote(440.0), "A4");     // A4 = 440 Hz
    EXPECT_EQ(frequencyToNote(220.0), "A3");     // A3 = 220 Hz
    EXPECT_EQ(frequencyToNote(880.0), "A5");     // A5 = 880 Hz
    EXPECT_EQ(frequencyToNote(261.63), "C4");    // Middle C
    EXPECT_EQ(frequencyToNote(329.63), "E4");    // E4
    EXPECT_EQ(frequencyToNote(493.88), "B4");    // B4
}

// Test guitar standard tuning frequencies
TEST_F(FrequencyToNoteTest, GuitarTuningFrequencies) {
    EXPECT_EQ(frequencyToNote(82.41), "E2");     // Low E
    EXPECT_EQ(frequencyToNote(110.0), "A2");     // A
    EXPECT_EQ(frequencyToNote(146.83), "D3");    // D
    EXPECT_EQ(frequencyToNote(196.0), "G3");     // G
    EXPECT_EQ(frequencyToNote(246.94), "B3");    // B
    EXPECT_EQ(frequencyToNote(329.63), "E4");    // High E
}

// Test rounding behavior (frequencies slightly off)
TEST_F(FrequencyToNoteTest, NearbyFrequencies) {
    EXPECT_EQ(frequencyToNote(439.5), "A4");     // Slightly flat A4
    EXPECT_EQ(frequencyToNote(440.5), "A4");     // Slightly sharp A4
    EXPECT_EQ(frequencyToNote(445.0), "A4");     // About 20 cents sharp
}

// Test frequencies below threshold
TEST_F(FrequencyToNoteTest, LowFrequencies) {
    EXPECT_EQ(frequencyToNote(15.0), "---");     // Below 20 Hz
    EXPECT_EQ(frequencyToNote(10.0), "---");
    EXPECT_EQ(frequencyToNote(0.0), "---");
}

// Test all 12 semitones in an octave
TEST_F(FrequencyToNoteTest, AllSemitones) {
    EXPECT_EQ(frequencyToNote(440.00), "A4");    // A
    EXPECT_EQ(frequencyToNote(466.16), "A#4");   // A#/Bb
    EXPECT_EQ(frequencyToNote(493.88), "B4");    // B
    EXPECT_EQ(frequencyToNote(523.25), "C5");    // C
    EXPECT_EQ(frequencyToNote(554.37), "C#5");   // C#/Db
    EXPECT_EQ(frequencyToNote(587.33), "D5");    // D
    EXPECT_EQ(frequencyToNote(622.25), "D#5");   // D#/Eb
    EXPECT_EQ(frequencyToNote(659.25), "E5");    // E
    EXPECT_EQ(frequencyToNote(698.46), "F5");    // F
    EXPECT_EQ(frequencyToNote(739.99), "F#5");   // F#/Gb
    EXPECT_EQ(frequencyToNote(783.99), "G5");    // G
    EXPECT_EQ(frequencyToNote(830.61), "G#5");   // G#/Ab
}

// Test octave range
TEST_F(FrequencyToNoteTest, DifferentOctaves) {
    EXPECT_EQ(frequencyToNote(27.50), "A0");     // Lowest piano key
    EXPECT_EQ(frequencyToNote(55.0), "A1");
    EXPECT_EQ(frequencyToNote(110.0), "A2");
    EXPECT_EQ(frequencyToNote(220.0), "A3");
    EXPECT_EQ(frequencyToNote(440.0), "A4");
    EXPECT_EQ(frequencyToNote(880.0), "A5");
    EXPECT_EQ(frequencyToNote(1760.0), "A6");
    EXPECT_EQ(frequencyToNote(3520.0), "A7");
}

// Test edge cases
TEST_F(FrequencyToNoteTest, EdgeCases) {
    EXPECT_EQ(frequencyToNote(20.0), "D#0");     // Just at threshold (19.45 Hz = D#0, 20.60 Hz = E0)
    EXPECT_EQ(frequencyToNote(19.9), "---");     // Just below threshold
}
