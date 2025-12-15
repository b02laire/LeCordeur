#pragma once
#include <vector>

#include "config.hpp"

struct ring_buffer {
    int head;
    int tail;
    int size;
    std::vector<Complex> buffer;

    explicit ring_buffer(int capacity);


    int write(const Complex* data, int count);

    int read(Complex* dest, int count);

    int available() const;

    int space() const;

    bool isEmpty() const;

    bool isFull() const;
};
