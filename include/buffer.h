#ifndef BUFFER_H
#define BUFFER_H
#include <sys/uio.h>
#include <vector>
#include <unistd.h>

#include <iostream>
#include <algorithm>

class Buffer {
public:
    Buffer(int buffer_size);
    ~Buffer() = default;
    int ReadFd(int fd);
    int WriteFd(int fd);
    // only for debug
    std::string RetrieveAllString();

private:
    std::vector<char> buffer_;
    size_t read_index_;
    size_t write_index_;
    
    size_t ReadableBytes() const;
    size_t WritableBytes() const;
    void MakeSpace(size_t len);
};
#endif