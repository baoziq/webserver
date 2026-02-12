#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H
#include <unordered_map>
#include <string>

#include "buffer.h"

class HttpRequest {
public:
    HttpRequest();
    ~HttpRequest();
    bool Parse(Buffer& buffer);
    // 解析完一条请求后重置状态，支持长连接多次请求
    void Reset();
    std::string GetMethod() const;
    std::string GetPath() const;
    std::string GetVersion() const;
    std::string GetHeader(const std::string& key) const;
    std::string GetBody() const;
    enum ParseState {
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH
    };
private:
    std::string method_;
    std::string path_;
    std::string version_;
    std::unordered_map<std::string, std::string> header_;
    std::string body_;
    bool ParseRequestLine(const std::string& buffer);
    bool ParseRequestHeader(const std::string& buffer);
    bool ParseRequestBody(const std::string& buffer);
    ParseState state_;
};
#endif