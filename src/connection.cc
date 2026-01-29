#include "../include/connection.h"

Connection::Connection(int fd, Epoller* epoller) : fd_(fd), epoller_(epoller), input_buffer_(1024), output_buffer_(1024), is_close_(false) {
    epoller_->AddFd(fd_, EPOLLIN);
}
Connection::~Connection() {
    epoller_->DelFd(fd_);
    close(fd_);
}

void Connection::Send(const std::string& str) {
    ssize_t n = 0;
    if (output_buffer_.ReadableBytes() == 0) {
        n = write(fd_, str.data(), str.size());
        if (n < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                SetClose();
                return;
            }
        }
        if (static_cast<int>(n) < str.size()) {
            output_buffer_.Append(str.data() + n, str.size() - n);
            epoller_->ModFd(fd_, EPOLLIN|EPOLLOUT);
        }

    }
}

// EPOLLOUT
void Connection::HandleWrite() {
    if (is_close_) {
        return ;
    }
    int n = output_buffer_.WriteFd(fd_);
    if (n < 0) {
        SetClose();
        return;
    }
    if (output_buffer_.ReadableBytes() == 0) {
        epoller_->ModFd(fd_, EPOLLIN);
    }

}

// EPOLLIN
void Connection::HandleRead() {
    int datas = input_buffer_.ReadFd(fd_);
    if (datas > 0) {
        std::string response = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 13\r\n"
            "\r\n"
            "Hello, Linux!";
        Send(response);
    } else if (datas == 0) {
        std::cout << "connection close\n";
        SetClose();
        close(fd_);
        return;
    } else {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        }
        perror("recv");
    }
}

void Connection::SetClose() {
    is_close_ = true;
}

bool Connection::IsClose() {
    return is_close_;
}