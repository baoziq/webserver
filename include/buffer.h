#include <sys/uio.h>
#include <vector>
#include <unistd.h>

#include <iostream>
#include <algorithm>

class Buffer {
public:
    Buffer(int buffer_size);
    ~Buffer() = default;
    bool ReadFd(int fd);
    bool WriteFd(int fd);

private:
    std::vector<char> buffer_;
    size_t read_index_;
    size_t write_index_;
    
    size_t ReadableBytes() const;
    size_t WritableBytes() const;
};