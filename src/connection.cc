#include "connection.h"

int setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        return -1;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

Connection::Connection(int fd, int epfd) : fd_(fd), epfd_(epfd), buf_{} {
    setNonBlocking(fd_);
    epoll_event cev{};
    cev.data.fd = fd_;
    cev.events = EPOLLIN;
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd_, &cev);
    std::cout << "new connection\n";
}

void Connection::closeConnection() {
    epoll_ctl(epfd_, EPOLL_CTL_DEL, fd_, nullptr);
    close(fd_);
    std::cout << "Connection is closed\n";
}

void Connection::HandleRead() {
    while (true) {
        std::cout << "recv messages\n";
        int bytes = recv(fd_, buf_, sizeof(buf_), 0);
        if (bytes > 0) {
            std::cout << "buf: " << buf_ << std::endl;
        } else if (bytes == 0) {
            closeConnection();
            return;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            perror("recv");
            closeConnection();
            return;
        }
    }
}