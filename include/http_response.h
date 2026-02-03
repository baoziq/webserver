#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <string>

class HttpResponse {
public:
    HttpResponse();
    ~HttpResponse() = default;

    void SetStatusCode(int code);  // 200, 404, 500
    void SetContentType(const std::string& type);
    void SetBody(const std::string& body);
    std::string Build();

private:
    int status_code_;
    std::string content_type_;
    std::string body_;
    std::string GetStatusMessage() const;
};

#endif