#include "../include/http_request.h"
#include <sstream>

HttpRequest::HttpRequest() : method_(), path_(), version_(),  header_(), body_(), state_(REQUEST_LINE) {}

HttpRequest::~HttpRequest() {}

std::string HttpRequest::GetMethod() const {
    return method_;
}

std::string HttpRequest::GetHeader(const std::string& key) const {
   auto it = header_.find(key);
   if (it == header_.end()) {
    return "";
   }
   return it->second;
}

std::string HttpRequest::GetPath() const {
    return path_;
}

std::string HttpRequest::GetVersion() const {
    return version_;
}

std::string HttpRequest::GetBody() const {
    return body_;
}

bool HttpRequest::ParseRequestLine(const std::string& line) {
    std::istringstream iss(line);
    if (!(iss >> method_ >> path_ >> version_)) {
        return false;
    }
    return true;
}

bool HttpRequest::ParseRequestHeader(const std::string& line) {
    size_t colon_pos = line.find(':');
    if (colon_pos == std::string::npos) {
        return false;
    }
    std::string key = line.substr(0, colon_pos);
    std::string value = line.substr(colon_pos + 1); // 头可能为空

    size_t start = value.find_first_not_of(" \t");
    size_t end = value.find_last_not_of(" \t\r");
    if (start != std::string::npos && end != std::string::npos) {
        value = value.substr(start, end - start + 1);
    } else {
        value = "";  // 全是空白字符
    }

    header_[key] = value;
    return true;
}

bool HttpRequest::ParseRequestBody(const std::string& line) {
    body_ = line;
    return true;
}

bool HttpRequest::Parse(Buffer& buffer) {
    std::string line;
    while (buffer.ReadLine(line)) {
        switch (state_) {
            case REQUEST_LINE:
                if (!ParseRequestLine(line)) {
                    return false;
                }
                state_ = HEADERS;
                break;
            case HEADERS:
                if (line.empty()) {
                    // 空行表示头部结束
                    state_ = BODY;
                    
                    // 检查是否有 Content-Length，如果有则读取 Body
                    auto it = header_.find("Content-Length");
                    if (it != header_.end()) {
                        try {
                            int length = std::stoi(it->second);
                            if (length > 0) {
                                if (buffer.ReadBytes(body_, length)) {
                                    state_ = FINISH;
                                    return true;
                                } else {
                                    // Body 数据还未完全接收，等待更多数据
                                    return false;
                                }
                            }
                        } catch (...) {
                            // Content-Length 格式错误
                            return false;
                        }
                    }
                    
                    // 没有 Content-Length（如 GET 请求），解析完成
                    state_ = FINISH;
                    return true;
                }
                if (!ParseRequestHeader(line)) {
                    return false;
                }
                break;
            case BODY:
            case FINISH:
                // 已经完成解析
                return true;
        }
    }
    
    // 如果已经完成或到达 BODY 状态（没有 body 的请求），返回 true
    return (state_ == BODY || state_ == FINISH);
}