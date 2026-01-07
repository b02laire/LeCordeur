#include <iostream>
#include <portaudio.h>
#include <vector>
#include <cmath>
#include <complex>
#include <thread>
#include "config.hpp"
#include "dsp.hpp"
#include "ring_buffer.hpp"


ring_buffer audioBuffer(FRAMES_PER_BUFFER * 10);

// Callback function for PortAudio
static int paCallback(
    const void* inputBuffer,
    void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData
){
    auto samples = static_cast<const float*>(inputBuffer);
    CArray frame(framesPerBuffer);

    for (unsigned long i = 0; i < framesPerBuffer; i++){
        frame[i] = {samples[i], 0.0};
    }

    audioBuffer.write(frame.data(), framesPerBuffer);

    return paContinue;
}


std::string frequencyToNote(double freq) {
    if (freq < 20.0) return "---";

    // A4 = 440 Hz, calculate semitones from A4
    double semitones = 12.0 * std::log2(freq / 440.0);
    int nearestSemitone = static_cast<int>(std::round(semitones));

    // Note names starting from A
    const std::string noteNames[] = {"A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#"};
    int noteIndex = ((nearestSemitone % 12) + 12) % 12;

    // Calculate octave (A4 is octave 4)
    int octave = 4 + (nearestSemitone + (nearestSemitone < 0 ? -11 : 0)) / 12;

    return noteNames[noteIndex] + std::to_string(octave);
}

void processFFT(){
    while (isRecording){
        if (audioBuffer.available() < FRAMES_PER_BUFFER) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        CArray frame(FRAMES_PER_BUFFER);
        audioBuffer.read(frame.data(), FRAMES_PER_BUFFER);

        applyHannWindow(frame);
        fft(frame);

        // Find peak in guitar frequency range (80-1200 Hz)
        int minBin = static_cast<int>(80.0 * frame.size() / SAMPLE_RATE);
        int maxBin = static_cast<int>(1200.0 * frame.size() / SAMPLE_RATE);
        maxBin = std::min(maxBin, static_cast<int>(frame.size() / 2));

        double maxMagnitude = 0.0;
        int maxIndex = minBin;

        for (int i = minBin; i < maxBin; i++) {
            double magnitude = std::abs(frame[i]);
            if (magnitude > maxMagnitude) {
                maxMagnitude = magnitude;
                maxIndex = i;
            }
        }

        // Parabolic interpolation for sub-bin precision
        double refinedBin = maxIndex;
        if (maxIndex > 0 && maxIndex < frame.size() / 2 - 1) {
            double alpha = std::abs(frame[maxIndex - 1]);
            double beta = std::abs(frame[maxIndex]);
            double gamma = std::abs(frame[maxIndex + 1]);
            double p = 0.5 * (alpha - gamma) / (alpha - 2.0 * beta + gamma);
            refinedBin = maxIndex + p;
        }

        // Convert bin index to frequency
        double fundamentalFreq = refinedBin * SAMPLE_RATE / frame.size();
        std::string noteName = frequencyToNote(fundamentalFreq);

        std::cout << "\rNote: " << noteName << " (" << fundamentalFreq << " Hz)           ";
        std::cout.flush();
    }
}

int main(){
    PaError err = Pa_Initialize();
    if (err != paNoError){
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << "\n";
        return 1;
    }

    // Open the stream with the callback
    PaStream* stream;
    err = Pa_OpenDefaultStream(
        &stream,
        1, // 1 input channel
        0, // 0 output channels
        paFloat32, // 32-bit float samples
        SAMPLE_RATE, // Sample rate
        FRAMES_PER_BUFFER, // Frames per buffer
        paCallback, // Callback function
        nullptr // User data (optional)
    );
    if (err != paNoError){
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << "\n";
        return 1;
    }

    err = Pa_StartStream(stream);

    if (err != paNoError){
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << "\n";
        return 1;
    }

    isRecording = true;
    std::thread processingThread(processFFT);

    std::cout << "Recording... Press Enter to stop.\n";
    std::cout << "Standard Tuning:\n";

    std::cout <<
        "E 82.41 Hz | A 110.00 Hz | D 146.83 Hz | G 196.00 Hz | B 246.94 Hz | E 329.63 Hz\n";


    std::cin.get(); // Wait for Enter

    isRecording = false; // Signal thread to stop
    processingThread.join(); // Wait for thread to finish


    Pa_Terminate();


    return 0;
}
