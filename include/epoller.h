#ifndef EPOLLER_H
#define EPOLLER_H
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>

class Epoller {
public:
    Epoller(int max_events);
    ~Epoller();
    bool AddFd(int fd, uint32_t event);
    bool DelFd(int fd);
    bool ModFd(int fd, uint32_t event);
    int Wait(int timeouts);
    int GetFd(size_t i);
    int GetEvents(size_t i);
private:
    int epfd_;
    std::vector<struct epoll_event> events_;
    
};
#endif