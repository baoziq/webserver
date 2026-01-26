#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/socket.h>

#include <iostream>

#include "buffer.h"
class Connection {
public:
    Connection(int fd, int epfd);
    ~Connection() = default;
    void HandleRead();
    void closeConnection();
    
private:
    int fd_;
    int epfd_;
    char buf_[1024];
    
    Buffer* read_buf_;
    Buffer* write_buf_;
};