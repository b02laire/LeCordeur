#pragma once
struct ring_buffer {
    int head;
    int tail;
    int size;

    explicit ring_buffer(int capacity);

    void write(int);
    void read(int);
};
