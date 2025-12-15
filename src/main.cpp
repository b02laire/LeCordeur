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


void processFFT(){
    while (isRecording){
        if (audioBuffer.available() < FRAMES_PER_BUFFER) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        CArray frame(FRAMES_PER_BUFFER);
        audioBuffer.read(frame.data(), FRAMES_PER_BUFFER);

        applyFlatTopWindow(frame);
        fft(frame);

        // Find the fundamental frequency (peak magnitude)
        double maxMagnitude = 0.0;
        int maxIndex = 0;

        for (int i = 1; i < frame.size() / 2; i++){
            // Start at 1 to skip DC, go to Nyquist
            double magnitude = std::abs(frame[i]);
            if (magnitude > maxMagnitude){
                maxMagnitude = magnitude;
                maxIndex = i;
            }
        }

        // Convert bin index to frequency
        double fundamentalFreq = maxIndex * SAMPLE_RATE / frame.size();

        std::cout << "\rFundamental: " << fundamentalFreq << " Hz           ";
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
