#ifndef CONNECTION_H
#define CONNECTION_H

#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/socket.h>

#include <iostream>

#include "buffer.h"
#include "epoller.h"

class Connection {
public:
    Connection(int fd, Epoller* epoller);
    ~Connection();

    void HandleWrite();
    void HandleRead();
    void SetClose();
    bool IsClose();
    void Send(const std::string& str);
    
private:
    int fd_;
    Epoller* epoller_;
    Buffer input_buffer_;
    Buffer output_buffer_;
    bool is_close_;
    
};

#endif