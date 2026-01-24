#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <iostream>

#include "connection.h"

// 从内核中取出当前fd的标志位，把非阻塞那位设置为1
int setNonBlocking1(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        return -1;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main() {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        return 1;
    }

    setNonBlocking1(listen_fd);

    // 允许 bind 一个“还在 TIME_WAIT 状态的端口”
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8001);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(listen_fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(listen_fd, 10) < 0) {
        perror("listen");
        return 1;
    }

    int epfd = epoll_create1(0);
    if (epfd < 0) {
        perror("epoll create1");
        return 1;
    }

    epoll_event ev{};
    ev.data.fd = listen_fd;
    ev.events = EPOLLIN;

    epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);

    epoll_event events[1024];
    char buf[1024];

    Connection* conn = nullptr;

    while (true) {
        int n = epoll_wait(epfd, events, 1024, -1);
        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;

            if (fd == listen_fd) {
                while (true) {
                    int client_fd = accept(fd, nullptr, nullptr);
                    if (client_fd < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;
                        }
                        perror("accept");
                        return 1;
                    }
                    conn = new Connection(client_fd, epfd);
                }
                
            } else if (events[i].events & EPOLLIN) {
                // 保证当前fd是由EPOLLIN唤醒，如果写else的话，events[i].events可能有其他值
                conn->handleRead();
            }
        }
    }

    close(listen_fd);
    close(epfd);
    return 0;

}