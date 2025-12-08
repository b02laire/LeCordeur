#include <iostream>
#include <portaudio.h>
#include <vector>
#include <cmath>
#include <complex>
#include <thread>
#include <queue>
#include <mutex>

#define SAMPLE_RATE 48000
#define FRAMES_PER_BUFFER 4096





using Complex = std::complex<double>;
using CArray = std::vector<Complex>;
std::atomic<bool> isRecording = false;
std::queue<CArray> fftQueue;
std::mutex queueMutex;

// Global vector to store audio samples
CArray audioBuffer;

// Callback function for PortAudio
static int paCallback(
    const void* inputBuffer,
    void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData
)
{
    auto samples = static_cast<const float*>(inputBuffer);
    CArray frame(framesPerBuffer);

    for(unsigned long i = 0; i < framesPerBuffer; i++){
        frame[i] = {samples[i], 0.0};
    }

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        fftQueue.push(frame);
    }

    return paContinue;
}

void applyHannWindow(CArray& samples){
    for (int i = 0; i < samples.size(); i++){
        double multiplier = 0.5 * (1 - cos(
            2 * M_PI * i / (FRAMES_PER_BUFFER - 1)));
        samples[i] *= multiplier;
    }
}

void fft(CArray& samples){
    int N = samples.size();

    if (N == 1) return;

    CArray even(N / 2);
    CArray odd(N / 2);

    for (int j = 0; j < N / 2; j++){
        even[j] = samples[2 * j];
        odd[j] = samples[2 * j + 1];
    }
    fft(even);
    fft(odd);

    for (int k = 0; k < N / 2-1; k++){

        //Complex t = exp(-2*M_1_PI*k/N) * odd[k];
        Complex t = odd[k] * std::polar(1.0, -2.0 * M_PI * k / N);
        samples[k] = even[k] + t;
        samples[k+N/2] = even[k] - t;
    }
}

void processFFT()
{
    while(isRecording)
    {
        CArray frame;
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            if(fftQueue.empty()) continue;
            frame = fftQueue.front();
            fftQueue.pop();
        }

        applyHannWindow(frame);
        fft(frame);

        // Find the fundamental frequency (peak magnitude)
        double maxMagnitude = 0.0;
        int maxIndex = 0;

        for(int i = 1; i < frame.size() / 2; i++){  // Start at 1 to skip DC, go to Nyquist
            double magnitude = std::abs(frame[i]);
            if(magnitude > maxMagnitude){
                maxMagnitude = magnitude;
                maxIndex = i;
            }
        }

        // Convert bin index to frequency
        double fundamentalFreq = maxIndex * SAMPLE_RATE / frame.size();

        std::cout << "Fundamental: " << fundamentalFreq << " Hz\n";
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
    err = Pa_StartStream(stream);

    std::cin.get();  // Wait for Enter

    isRecording = false;  // Signal thread to stop
    processingThread.join();  // Wait for thread to finish


    Pa_Terminate();



    return 0;
}
