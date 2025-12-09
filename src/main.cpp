#include <iostream>
#include <portaudio.h>
#include <vector>
#include <cmath>
#include <complex>
#include <thread>
#include <queue>
#include <mutex>
#include "config.hpp"
#include "dsp.hpp"


std::queue<CArray> fftQueue;
std::mutex queueMutex;

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

    {
        std::lock_guard lock(queueMutex);
        fftQueue.push(frame);
    }

    return paContinue;
}


void processFFT(){
    while (isRecording){
        CArray frame;
        {
            std::lock_guard lock(queueMutex);
            if (fftQueue.empty()) continue;
            frame = fftQueue.front();
            fftQueue.pop();
        }

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

    // Print available devices
    int numDevices = Pa_GetDeviceCount();
    for (int i = 0; i < numDevices; i++){
        const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);
        std::cout << i << ": " << deviceInfo->name << "\n";
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

    isRecording = true;
    std::thread processingThread(processFFT);

    std::cout << "Recording... Press Enter to stop.\n";
    std::cout << "Standard Tuning:\n";

    std::cout <<
        "E 82.41 Hz | A 110.00 Hz | D 146.83 Hz | G 196.00 Hz | B 246.94 Hz | E 329.63 Hz\n";
    err = Pa_StartStream(stream);

    std::cin.get(); // Wait for Enter

    isRecording = false; // Signal thread to stop
    processingThread.join(); // Wait for thread to finish


    Pa_Terminate();


    return 0;
}
