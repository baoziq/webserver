#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <iostream>
#include <unordered_map>

#include "connection.h"
#include "buffer.h"
#include "epoller.h"

// 从内核中取出当前fd的标志位，把非阻塞那位设置为1
int setNonBlocking1(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        return -1;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

std::unordered_map<int, Connection*> client;

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
    
    Epoller* epoller = new Epoller(1024);
    epoller->AddFd(listen_fd, EPOLLIN);

    Buffer* buffer = new Buffer(8);

    // Connection* conn = nullptr;

    while (true) {
        int n = epoller->Wait(-1);
        for (size_t i = 0; i < n; i++) {
            int fd = epoller->GetFd(i);
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
                    client[client_fd] = new Connection(client_fd, epoller);
                }
                
            } else {
                auto it = client.find(fd);
                if (it == client.end()) {
                    continue;
                }
                Connection* conn = it->second;
                uint32_t events = epoller->GetEvents(i);
                if (events & EPOLLIN) {
                    conn->HandleRead();
                }
                if (events & EPOLLOUT) {
                    conn->HandleWrite();
                }
                if (conn->IsClose()) {
                    delete conn;
                    client.erase(it);
                }
                
            }
        }
    }

    close(listen_fd);
    return 0;

}