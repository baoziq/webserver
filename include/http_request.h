#include "buffer.h"

class HttpRequest {
public:
    HttpRequest();
    ~HttpRequest();
    bool Parse(Buffer& buf);
    std::string GetMethod() const;
    std::string GetPath() const;
    std::string GetVersion() const;
    std::string GetHeader(const std::string& key) const;

private:
    std::string method_;
    std::string path_;
    std::string version_;
    std::unordered_map<std::string, std::string> header_;
    std::string body_;
    bool ParseRequestLine(const std::string& str);
    bool ParseRequestHeader(const std::string& str);
    bool ParseRequestBody(const std::string& str);
};