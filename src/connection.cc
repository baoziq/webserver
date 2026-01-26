#include "connection.h"

Connection::Connection(int fd, Epoller* epoller) : fd_(fd), epoller_(epoller), input_buffer_(1024), output_buffer_(1024), is_close_(false) {
    epoller_->AddFd(fd_, EPOLLIN);
}
Connection::~Connection() {
    epoller_->DelFd(fd_);
}

// EPOLLOUT
void Connection::HandleWrite() {

}

// EPOLLIN
void Connection::HandleRead() {
    int datas = input_buffer_.ReadFd(fd_);
    if (datas > 0) {
        std::cout << "recv message\n";
        input_buffer_.WriteFd(fd_);

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