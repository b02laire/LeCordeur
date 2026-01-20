#include "../include/note_detection.hpp"
#include <cmath>
#include <string>

std::string frequencyToNote(double freq) {
    if (freq < 20.0) return "---";

    // A4 = 440 Hz, calculate semitones from A4
    double semitones = 12.0 * std::log2(freq / 440.0);
    int nearestSemitone = static_cast<int>(std::round(semitones));

    // Note names starting from C
    const std::string noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

    // Convert semitones from A4 to semitones from C0
    // A4 = 440 Hz is 9 semitones above C4, and C4 is 48 semitones above C0
    int semitonesFromC0 = nearestSemitone + 48 + 9;

    int noteIndex = ((semitonesFromC0 % 12) + 12) % 12;
    int octave = (semitonesFromC0 + (semitonesFromC0 < 0 ? -11 : 0)) / 12;

    return noteNames[noteIndex] + std::to_string(octave);
}
