#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/socket.h>

#include <iostream>

class Connection {
public:
    Connection(int fd, int epfd);
    ~Connection() = default;
    void handleRead();
    
private:
    int fd_;
    int epfd_;
    char buf_[1024];
    void closeConnection();
};