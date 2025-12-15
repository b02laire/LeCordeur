#include "ring_buffer.hpp"
#include <algorithm>
#include <cstring>

ring_buffer::ring_buffer(int capacity)
    : buffer(capacity)
      , size(capacity)
      , head(0)
      , tail(0){
}

int ring_buffer::write(const Complex* data, int count){
    int available_space = space();
    int to_write = std::min(count, available_space);

    if (to_write == 0){
        return 0;
    }
    int first_chunk = std::min(to_write, size - tail);
    std::copy(data, data + first_chunk, buffer.begin() + tail);

    if (to_write > first_chunk){
        int second_chunk = to_write - first_chunk;
        std::copy(data + first_chunk, data + first_chunk + second_chunk,
                  buffer.begin());
    }

    tail = (tail + to_write) % size;
    return to_write;
}

int ring_buffer::read(Complex* dest, int count){
    int available_data = available();
    int to_read = std::min(count, available_data);

    if (to_read == 0){
        return 0;
    }
    int first_chunk = std::min(to_read, size - head);
    std::copy(buffer.begin() + head, buffer.begin() + head + first_chunk,
              dest);

    if (to_read > first_chunk){
        int second_chunk = to_read - first_chunk;
        std::copy(buffer.begin(), buffer.begin() + second_chunk,
                  dest + first_chunk);
    }

    head = (head + to_read) % size;
    return to_read;
}

int ring_buffer::available() const{
    if (tail >= head){
        return tail - head;
    }
    else{
        return size - head + tail;
    }
}

int ring_buffer::space() const{
    return size - available() - 1;
}

bool ring_buffer::isEmpty() const{
    return head == tail;
}

bool ring_buffer::isFull() const{
    return (tail + 1) % size == head;
}
