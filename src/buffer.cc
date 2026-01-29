#include "../include/buffer.h"

Buffer::Buffer(int buffer_size) : buffer_(buffer_size), read_index_(0), write_index_(0) {}

size_t Buffer::ReadableBytes() const {
    return write_index_ - read_index_;
}

size_t Buffer::WritableBytes() const {
    return buffer_.size() - write_index_;
}

void Buffer::EnsureWritable(size_t len) {
    if (len > WritableBytes()) {
        MakeSpace(len);
    }
}

void Buffer::Append(const char* ch, size_t len) {
    EnsureWritable(len);
    std::copy(ch, ch + len, &buffer_[write_index_]);
    write_index_ +=  len;
}

void Buffer::Append(const std::string& str) {
    Append(str.data(), str.size());
}

int Buffer::ReadFd(int fd) {
    char buffer[65534];
    struct iovec iov[2];
    const int writable = WritableBytes();

    iov[0].iov_base = &buffer_[write_index_];
    iov[0].iov_len = WritableBytes();
    iov[1].iov_base = buffer;
    iov[1].iov_len = sizeof(buffer);

    int iovcnt = (writable < sizeof(buffer) ? 2 : 1);
    ssize_t len = readv(fd, iov, iovcnt);

    if (len < 0) {
        return -1;
    } else if (static_cast<size_t>(len) <= WritableBytes()) {
        write_index_ += len;
    } else {
        ssize_t remain_len = len - writable;  
        Append(buffer, remain_len);
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

void Buffer::MakeSpace(size_t len) {
    if (WritableBytes() + read_index_ < len) {
        buffer_.resize(write_index_ + len);
    } else {
        std::copy_backward(&buffer_[read_index_], &buffer_[write_index_], &buffer_[0]);
        write_index_ = write_index_ - read_index_;
        read_index_ = 0;
    }
}
