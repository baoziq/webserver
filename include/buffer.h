#include <vector>

class Buffer {
public:
    Buffer(int len);
    ~Buffer() = default;
    void ReadFd(int fd);
    void WriteFd(int fd);

private:
    std::vector<char> buf_;
    int read_index_;
    int write_index_;
};