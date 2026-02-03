#include "../include/connection.h"

Connection::Connection(int fd, Epoller* epoller) : 
    fd_(fd), epoller_(epoller), input_buffer_(1024), 
    output_buffer_(1024), is_close_(false),  
    http_request_() {
    epoller_->AddFd(fd_, EPOLLIN);
}
Connection::~Connection() {
    epoller_->DelFd(fd_);
    close(fd_);
}

void Connection::Send(const std::string& str) {
    ssize_t n = 0;
    if (output_buffer_.ReadableBytes() == 0) {
        n = write(fd_, str.data(), str.size());
        if (n < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                SetClose();
                return;
            }
        }
        if (static_cast<int>(n) < str.size()) {
            output_buffer_.Append(str.data() + n, str.size() - n);
            epoller_->ModFd(fd_, EPOLLIN|EPOLLOUT);
        }

    }
}

// EPOLLOUT
void Connection::HandleWrite() {
    if (is_close_) {
        return ;
    }
    int n = output_buffer_.WriteFd(fd_);
    if (n < 0) {
        SetClose();
        return;
    }
    if (output_buffer_.ReadableBytes() == 0) {
        epoller_->ModFd(fd_, EPOLLIN);
    }

}

// EPOLLIN
void Connection::HandleRead() {
    int datas = input_buffer_.ReadFd(fd_);
    if (datas > 0) {
        if (!http_request_.Parse(input_buffer_)) {
            return;
        }
        HttpResponse response;
        response.SetStatusCode(200);
        response.SetContentType("text/plain");
        std::string body = "Method: " + http_request_.GetMethod() + "\n"
                        + "Path: " + http_request_.GetPath() + "\n"
                        + "Version: " + http_request_.GetVersion() + "\n"
                        + "Body: " + http_request_.GetBody();
        response.SetBody(body);
        Send(response.Build());
    } else if (datas == 0) {
        SetClose();
        return;
    } else {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        }
        perror("recv");
    }
}

void Connection::SetClose() {
    is_close_ = true;
}

bool Connection::IsClose() {
    return is_close_;
}