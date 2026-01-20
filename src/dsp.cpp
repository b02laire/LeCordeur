#include "../include/dsp.hpp"
#include <cmath>

void applyHannWindow(CArray& samples){
    for (int i = 0; i < samples.size(); i++){
        double multiplier = 0.5 * (1 - cos(
            2 * M_PI * i / (samples.size() - 1)));
        samples[i] *= multiplier;
    }
}

void applyFlatTopWindow(CArray& samples){
    /*
    Using coefficients from the matlab implementation:
    https://www.mathworks.com/help/signal/ref/flattopwin.html
    */
    for (int i = 0; i < samples.size(); i++){
        double multiplier = 0.21557895
            - 0.41663158 * cos(2 * M_PI * i / (samples.size() - 1))
            + 0.277263158 * cos(4 * M_PI * i / (samples.size() - 1))
            - 0.083578947 * cos(6 * M_PI * i / (samples.size() - 1))
            + 0.006947368 * cos(8 * M_PI * i / (samples.size() - 1));
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

    for (int k = 0; k <= N / 2 - 1; k++){
        //Complex t = exp(-2*M_1_PI*k/N) * odd[k];
        Complex t = odd[k] * std::polar(1.0, -2.0 * M_PI * k / N);
        samples[k] = even[k] + t;
        samples[k + N / 2] = even[k] - t;
    }
}