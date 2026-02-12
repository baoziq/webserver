#include "../include/buffer.h"
#include <cstddef>
#include <unistd.h>

Buffer::Buffer(const int buffer_size) : buffer_(buffer_size), read_index_(0), write_index_(0) {}

size_t Buffer::ReadableBytes() const {
    return write_index_ - read_index_;
}

size_t Buffer::WritableBytes() const {
    return buffer_.size() - write_index_;
}

void Buffer::EnsureWritable(const size_t len) {
    if (len > WritableBytes()) {
        MakeSpace(len);
    }
}

void Buffer::Append(const char* ch, const size_t len) {
    EnsureWritable(len);
    std::copy(ch, ch + len, &buffer_[write_index_]);
    write_index_ +=  len;
}

void Buffer::Append(const std::string& str) {
    Append(str.data(), str.size());
}

int Buffer::ReadFd(const int fd) {
    char extrabuf[65536];
    struct iovec iov[2];
    const size_t writable = WritableBytes();

    iov[0].iov_base = &buffer_[write_index_];
    iov[0].iov_len = writable;
    iov[1].iov_base = extrabuf;
    iov[1].iov_len = sizeof(extrabuf);

    // 如果 buffer_ 空间足够大，就不用 extrabuf
    const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
    const ssize_t len = readv(fd, iov, iovcnt);

    if (len < 0) {
        return -1;
    } else if (static_cast<size_t>(len) <= writable) {
        write_index_ += len;
    } else {
        // buffer_ 被填满，剩余数据在 extrabuf 里
        write_index_ = buffer_.size();  // 先把 buffer_ 标记为满
        Append(extrabuf, len - writable);
    }
    return len;
}

int Buffer::WriteFd(const int fd) {
    const ssize_t len = write(fd, &buffer_[read_index_], ReadableBytes());
    if (len < 0) {
        perror("write");
        return -1;
    }
    read_index_ += len;
    return len;
}

void Buffer::MakeSpace(const size_t len) {
    if (WritableBytes() + read_index_ < len) {
        buffer_.resize(write_index_ + len);
    } else {
        std::copy(&buffer_[read_index_], &buffer_[write_index_], &buffer_[0]);
        write_index_ = write_index_ - read_index_;
        read_index_ = 0;
    }
}

bool Buffer::ReadLine(std::string& line) {
    // 至少需要 2 字节才能包含 \r\n
    if (ReadableBytes() < 2) {
        return false;
    }
    for (size_t i = read_index_; i + 1 < write_index_; ++i) {
        if (buffer_[i] == '\r' && buffer_[i + 1] == '\n') {
            line.assign(&buffer_[read_index_], i - read_index_);
            read_index_ = i + 2;
            return true;
        }
    }
    return false;
}

bool Buffer::ReadBytes(std::string& data, const size_t len) {
    if (ReadableBytes() < len) {
        return false;  // 数据不够
    }
    data.assign(&buffer_[read_index_], len);
    read_index_ += len;
    return true;
}
