#include <sys/epoll.h>
#include <vector>

class Epoller {
public:
    Epoller(int max_events);
    ~Epoller() = default;
    bool AddFd(int fd, uint32_t event);
    bool DelFd(int fd, uint32_t event);
    bool ModFd(int fd, uint32_t event);
private:
    int epfd_;
    std::vector<struct epoll_event> events_;
};