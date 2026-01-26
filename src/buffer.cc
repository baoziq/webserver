#include "buffer.h"

Buffer::Buffer(int buffer_size) : buffer_(buffer_size), read_index_(0), write_index_(0) {}

size_t Buffer::ReadableBytes() const {
    return write_index_ - read_index_;
}

size_t Buffer::WritableBytes() const {
    return buffer_.size() - write_index_;
}

int Buffer::ReadFd(int fd) {
    char buffer[65534];
    struct iovec iov[2];
    iov[0].iov_base = &buffer_[write_index_];
    iov[0].iov_len = WritableBytes();
    iov[1].iov_base = buffer;
    iov[1].iov_len = sizeof(buffer);
    
    ssize_t len = readv(fd, iov, 2);
    if (len < 0) {
        perror("readv");
        return -1;
    } else if (static_cast<size_t>(len) <= WritableBytes()) {
        write_index_ += len;
    } else {
        ssize_t remain_len = len - WritableBytes();
        write_index_ = buffer_.size();
        if (remain_len > read_index_) {
            buffer_.resize(buffer_.size() + remain_len + 1);
            std::copy(&buffer[0], &buffer[remain_len], &buffer_[write_index_]);
        } else {
            std::copy(&buffer_[read_index_], &buffer_[write_index_], &buffer_[0]);
            read_index_ = 0;
            write_index_ = buffer_.size();
            std::copy(&buffer[0], &buffer[remain_len], &buffer_[write_index_]);
        }
        write_index_ += remain_len;
    }
    for (int i = 0; i < len; i++) {
        std::cout << "buffer: " << buffer_[i] << std::endl;
    }
    return len;
}

int Buffer::WriteFd(int fd) {
    ssize_t len = write(fd, &buffer_[read_index_], ReadableBytes());
    if (len < 0) {
        perror("write");
        return -1;
    }
    read_index_ += len;
    return len;
}

std::string Buffer::RetrieveAllString() {
    std::string res(buffer_.begin() + read_index_, buffer_.begin() + write_index_);
    read_index_ = 0;
    write_index_ = 0;
    return res;
}
