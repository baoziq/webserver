#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cerrno>

#include <unordered_map>
#include <memory>

#include "../include/connection.h"
#include "../include/epoller.h"

// 从内核中取出当前 fd 的标志位，把非阻塞那一位设置为 1
int setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        return -1;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

// 使用智能指针管理连接，避免手动 delete
std::unordered_map<int, std::unique_ptr<Connection>> clients;

int main() {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        return 1;
    }

    if (setNonBlocking(listen_fd) < 0) {
        perror("fcntl listen_fd");
        close(listen_fd);
        return 1;
    }

    // 允许 bind 一个“还在 TIME_WAIT 状态的端口”
    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(listen_fd);
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8001);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(listen_fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        perror("bind");
        close(listen_fd);
        return 1;
    }

    if (listen(listen_fd, 128) < 0) {
        perror("listen");
        close(listen_fd);
        return 1;
    }
    
    Epoller epoller(1024);
    if (!epoller.AddFd(listen_fd, EPOLLIN)) {
        perror("epoll add listen_fd");
        close(listen_fd);
        return 1;
    }

    while (true) {
        int n = epoller.Wait(-1);
        if (n < 0) {
            if (errno == EINTR) {
                continue;   // 被信号打断，重新等待
            }
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < n; ++i) {
            int fd = epoller.GetFd(i);
            uint32_t events = epoller.GetEvents(i);

            if (fd == listen_fd) {
                // 处理所有就绪的连接
                while (true) {
                    int client_fd = accept(listen_fd, nullptr, nullptr);
                    if (client_fd < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;
                        }
                        perror("accept");
                        break;
                    }

                    if (setNonBlocking(client_fd) < 0) {
                        perror("fcntl client_fd");
                        close(client_fd);
                        continue;
                    }

                    // 创建连接对象，并交给 epoller 管理
                    clients[client_fd] = std::make_unique<Connection>(client_fd, &epoller);
                }
            } else {
                auto it = clients.find(fd);
                if (it == clients.end()) {
                    continue;
                }
                Connection* conn = it->second.get();

                // 处理错误 / 挂断事件
                if (events & (EPOLLERR | EPOLLHUP)) {
                    conn->SetClose();
                } else {
                    if (events & EPOLLIN) {
                        conn->HandleRead();
                    }
                    if (events & EPOLLOUT) {
                        conn->HandleWrite();
                    }
                }

                if (conn->IsClose()) {
                    clients.erase(it);
                }
            }
        }
    }

    close(listen_fd);
    clients.clear();
    return 0;
}