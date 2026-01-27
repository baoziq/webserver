#include "http_request.h"

HttpRequest::HttpRequest() : method_(), path_(), version_(),  header_(), body_() {}

std::string HttpRequest::GetMethod() const {
    return method_;
}

std::string HttpRequest::GetHeader(const std::string& key) const {
    return header_[key];
}

std::string HttpRequest::GetPath() const {
    return path_;
}