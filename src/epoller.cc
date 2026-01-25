#include "epoller.h"

Epoller::Epoller(int max_events) : epfd_(epoll_create1(0)), events_(max_events) {}

bool Epoller::AddFd(int fd, uint32_t events) {
    epoll_event ev{};
    ev.data.fd = fd;
    ev.events = events;
    return epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev) == 0;   // epoll_ctl成功会返回0
}

bool Epoller::DelFd(int fd, uint32_t events) {
    return epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr) == 0;
}

bool Epoller::ModFd(int fd, uint32_t events) {
    epoll_event ev{};
    ev.data.fd = fd;
    ev.events = events;
    return epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev) == 0;
}