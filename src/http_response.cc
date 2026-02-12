#include "../include/http_response.h"
#include <sstream>

HttpResponse::HttpResponse() : status_code_(200), content_type_("text/plain"), body_() {}

void HttpResponse::SetStatusCode(int code) {
    status_code_ = code;
}

void HttpResponse::SetContentType(const std::string& type) {
    content_type_ = type;
}

void HttpResponse::SetBody(const std::string& body) {
    body_ = body;
}

std::string HttpResponse::GetStatusMessage() const {
    switch (status_code_) {
        case 200: return "OK";
        case 400: return "Bad Request";
        case 404: return "Not Found";
        case 500: return "Internal Server Error";
        default: return "Unknown";
    }
}

std::string HttpResponse::Build() {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << status_code_ << " " << GetStatusMessage() << "\r\n";
    oss << "Content-Type: " << content_type_ << "\r\n";
    oss << "Content-Length: " << body_.size() << "\r\n";
    // 默认使用长连接，便于压测工具复用 TCP 连接
    oss << "Connection: keep-alive\r\n";
    oss << "\r\n";
    oss << body_;
    return oss.str();
}
