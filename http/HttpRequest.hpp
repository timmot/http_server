// Copyright (c) 2022 - Tim Blackstone

#pragma once

#include "../utility/Bytes.hpp"
#include "../utility/log.hpp"
#include <string>
#include <unordered_map>

enum class Method {
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    CONNECT,
    OPTIONS,
    TRACE,
    PATCH
};

struct HttpRequest {
    Method method;
    std::string path;
    std::unordered_map<std::string, std::string> headers;

    std::string method_name()
    {
        switch (method) {
        case Method::GET:
            return "GET";
        case Method::HEAD:
            return "HEAD";
        case Method::POST:
            return "POST";
        case Method::PUT:
            return "PUT";
        case Method::DELETE:
            return "DELETE";
        case Method::CONNECT:
            return "CONNECT";
        case Method::OPTIONS:
            return "OPTIONS";
        case Method::TRACE:
            return "TRACE";
        case Method::PATCH:
            return "PATCH";
        default:
            return "";
        }
    }

    inline void debug()
    {
        logln("{} request to {}", method_name(), path);

        logln("Headers:");
        for (auto pair : headers) {
            logln("{}:{}", pair.first, pair.second);
        }
    }
};