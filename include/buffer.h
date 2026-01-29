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
    size_t ReadableBytes() const;
    size_t WritableBytes() const;
    void Append(const char* ch, size_t len);
    void Append(const std::string& str);
    void EnsureWritable(size_t len);
    bool ReadLine(std::string& line);
    bool ReadBytes(std::string& data, size_t len);
private:
    std::vector<char> buffer_;
    size_t read_index_;
    size_t write_index_;
    
    
    void MakeSpace(size_t len);
};
#endif