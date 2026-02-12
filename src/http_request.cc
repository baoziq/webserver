#include "../include/http_request.h"
#include <sstream>
#include <cctype>
#include <algorithm>

HttpRequest::HttpRequest() : method_(), path_(), version_(),  header_(), body_(), state_(REQUEST_LINE) {}

HttpRequest::~HttpRequest() {}

void HttpRequest::Reset() {
    method_.clear();
    path_.clear();
    version_.clear();
    header_.clear();
    body_.clear();
    state_ = REQUEST_LINE;
}

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

// Content-Length 头大小写不敏感查找
static std::string GetContentLength(const std::unordered_map<std::string, std::string>& h) {
    for (const auto& p : h) {
        if (p.first.size() == 14 &&
            std::equal(p.first.begin(), p.first.end(), "Content-Length",
                [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b)); }))
            return p.second;
    }
    return "";
}

bool HttpRequest::Parse(Buffer& buffer) {
    // 若已在 BODY 状态（上次数据不足），直接尝试读取
    if (state_ == BODY) {
        std::string cl = GetContentLength(header_);
        if (!cl.empty()) {
            try {
                int len = std::stoi(cl);
                if (len > 0 && buffer.ReadableBytes() >= static_cast<size_t>(len)) {
                    if (buffer.ReadBytes(body_, static_cast<size_t>(len))) {
                        state_ = FINISH;
                        return true;
                    }
                }
            } catch (...) {}
        }
        return false;
    }
    std::string line;

    while (buffer.ReadLine(line)) {

        switch (state_) {
            case REQUEST_LINE:
                if (!ParseRequestLine(line)) {
                    return false;
                }
                state_ = HEADERS;
                break;
            case HEADERS: {
                if (line.empty()) {
                    // 空行表示头部结束，按 Content-Length 读取 body
                    std::string cl = GetContentLength(header_);
                    if (cl.empty()) {
                        state_ = FINISH;
                        return true;  // 无 body（如 GET）
                    }
                    try {
                        int len = std::stoi(cl);
                        if (len <= 0) {
                            state_ = FINISH;
                            return true;
                        }
                        if (buffer.ReadableBytes() < static_cast<size_t>(len)) {
                            state_ = BODY;
                            return false;  // 数据未收齐，下次继续
                        }
                        if (!buffer.ReadBytes(body_, static_cast<size_t>(len))) {
                            return false;
                        }
                        state_ = FINISH;
                        return true;
                    } catch (...) {
                        return false;  // Content-Length 格式错误
                    }
                }
                if (!ParseRequestHeader(line)) {
                    return false;
                }
                break;
            }
            case BODY:
            case FINISH:
                return true;  // BODY 已在 Parse 开头处理，FINISH 表示完成
        }
    }
    return (state_ == BODY || state_ == FINISH);
}