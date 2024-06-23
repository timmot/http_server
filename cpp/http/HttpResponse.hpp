// Copyright (c) 2022 - Tim Blackstone

#pragma once

#include "../utility/Bytes.hpp"
#include "../utility/File.hpp"
#include "../utility/MimeTypeDatabase.hpp"
#include "../utility/log.hpp"
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

struct HttpResponse {
    std::string http_version;
    std::string status_code;
    std::string reason_phrase;
    std::unordered_map<std::string, std::string> headers;
    std::optional<Bytes> body;

    static std::optional<HttpResponse> create_file_response(std::string_view filename)
    {
        MimeTypeDatabase mime_type_db;
        HttpResponse response = {};
        response.http_version = "1.1";
        response.status_code = "200";
        response.reason_phrase = "OK";
        response.headers.emplace("Connection", "close");

        auto maybe_mime_type = mime_type_db.detect_mime_type(filename);
        if (maybe_mime_type.has_value())
            response.headers.emplace("Content-Type", *maybe_mime_type);

        auto maybe_file = File::open(filename);
        if (!maybe_file.has_value()) {
            logln("HttpResponse: Failed to open file \"{}\"", filename);
            return {};
        }
        auto file_bytes = (*maybe_file)->read_all();
        response.headers.emplace("Content-Length", std::to_string(file_bytes.size()));
        response.body = file_bytes;

        return response;
    }

    static HttpResponse create_error_response()
    {
        HttpResponse response = {};
        response.http_version = "1.1";
        response.status_code = "500";
        response.reason_phrase = "Internal Server Error";
        response.headers.emplace("Connection", "close");
        response.headers.emplace("Content-Type", "text/plain");
        response.body = Bytes { "500 Internal Server Error\n", 26 };
        response.headers.emplace("Content-Length", std::to_string(response.body->size()));
        return response;
    }

    std::string serialise()
    {
        std::string output = "HTTP/";
        output += http_version + " " + status_code + " " + reason_phrase + "\n";

        for (auto const& entry : headers) {
            output += entry.first + ": " + entry.second + "\n";
        }

        output += "\n";

        if (body.has_value()) {
            output += std::string { (char*)body->data(), body->size() };
        }

        return output;
    }
};
