// Copyright (c) 2022 - Tim Blackstone

#include "utility/TcpServer.h"
#include "utility/Bytes.hpp"
#include "utility/EventLoop.h"
#include "utility/Socket.h"
#include "utility/String.hpp"
#include "utility/log.hpp"
#include <iostream>
#include <memory>
#include <signal.h>
#include <stdio.h>
#include <string_view>

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

struct HttpResponse {
    std::string http_version = "1.1";
    std::string status_code = "200";
    std::string reason_phrase = "OK";
    std::unordered_map<std::string, std::string> headers = {
        { "Connection", "close" }
    };

    void serialise(Bytes& buffer)
    {
        int string_length = std::snprintf((char*)buffer.data(), 1024, "HTTP/%s %s %s\n%s: %s\n\n", http_version.c_str(), status_code.c_str(), reason_phrase.c_str(), "Connection", headers.at("Connection").c_str());
        buffer.trim(string_length);
    }
};

/* RFC9112 - 3.
     HTTP-message   = start-line
                      *( header-field CRLF )
                      CRLF
                      [ message-body ]
*/

// The normal procedure for parsing an HTTP message is to read the start-line into a structure, read each header field line into a hash table by field name until the empty line, and then use the parsed data to determine if a message body is expected. If a message body has been indicated, then it is read as a stream until an amount of octets equal to the message body length is read or the connection is closed.

// https://www.rfc-editor.org/rfc/rfc9110
// https://www.rfc-editor.org/rfc/rfc9112
// https://rfcs.io/http

// Re: Managing partial messages in a Keep-Alive
// If a valid Content-Length header field is present without Transfer-Encoding, its decimal value defines the expected message body length in octets. If the sender closes the connection or the recipient times out before the indicated number of octets are received, the recipient MUST consider the message to be incomplete and close the connection.

// https://www.rfc-editor.org/rfc/rfc9112#name-message-body-length

// https://www.rfc-editor.org/rfc/rfc9112#name-persistence
[[nodiscard]] std::optional<HttpRequest>
parse_html(std::string const& request)
{
    // TODO: Exchange this for socket code
    // TODO: Clear all excess \r (CR) as in spec.
    auto lines = split_view(request, '\n');
    size_t read_lines = 0;
    auto get_line = [&lines, &read_lines]() {
        if (read_lines >= lines.size())
            return std::optional<std::string_view> {};
        return std::optional { lines[read_lines++] };
    };

    auto request_line = split_view(*get_line(), ' ');
    if (request_line.size() != 3) {
        logln("Wrong amount of args in request line");
        exit(1);
    }

    std::unordered_map<std::string, std::string> headers;
    for (;;) {
        auto header_line = get_line();

        if (!header_line.has_value() || header_line == "\r")
            break;

        // FIXME: Doesn't handle multi-occurring headers
        // FIXME: Doesn't handle list value headers
        auto header_parts = split_view(*header_line, ':');

        if (header_parts.size() < 2) {
            logln("Header line has no colon");
            continue;
        }

        auto header_colon = header_line->find_first_of(':') + 1;
        auto header_value_length = header_line->size() - 1 - header_colon;
        headers.emplace(header_parts[0], header_line->substr(header_colon, header_value_length));
    }

    // TODO: Read body?

    HttpRequest http_request = {};
    auto method = request_line[0];
    if (method == "GET")
        http_request.method = Method::GET;
    else if (method == "HEAD")
        http_request.method = Method::HEAD;
    else if (method == "POST")
        http_request.method = Method::POST;
    else if (method == "PUT")
        http_request.method = Method::PUT;
    else if (method == "DELETE")
        http_request.method = Method::DELETE;
    else if (method == "CONNECT")
        http_request.method = Method::CONNECT;
    else if (method == "OPTIONS")
        http_request.method = Method::OPTIONS;
    else if (method == "TRACE")
        http_request.method = Method::TRACE;
    else if (method == "PATCH")
        http_request.method = Method::PATCH;
    else {
        logln("Invalid method in request line");
        exit(1);
    }

    // NOTE: This is the request target, it can come in four different forms depending on the method
    http_request.path = request_line[1];
    http_request.headers = std::move(headers);

    return http_request;
}

int main(int argc, char* argv[])
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s host port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    auto server = TcpServer::create();

    if (!server.has_value())
        exit(EXIT_FAILURE);

    EventLoop loop;

    if (server->listen(argv[1], atoi(argv[2])))
        logln("listening on port: {}", argv[2]);

    std::vector<std::unique_ptr<Socket>> clients;
    server->on_read = [&server, &clients](EventLoop& loop) {
        clients.emplace_back(server->accept());
        auto client_id = clients.size() - 1;
        logln("Accepted new client {}", clients.back()->fd());

        loop.add_read(clients.back()->fd(), [&clients, client_id](EventLoop& loop) {
            Bytes buf(1024);
            auto nread = clients[client_id]->read(buf);

            if (nread <= 0) {
                logln("Connection closed! {}", nread);
                loop.remove_read(clients[client_id]->fd());
                return;
            }

            std::string message((char*)buf.data(), nread);
            auto request = parse_html(message);

            if (request) {
                request->debug();
            }

            HttpResponse response = {};
            response.http_version = "1.1";
            response.status_code = "200";
            response.reason_phrase = "OK";
            response.headers.emplace("Connection", "close");

            Bytes response_buffer(1024);
            response.serialise(response_buffer);
            logln("goodbye crule world");
            std::string a((char*)response_buffer.data(), response_buffer.size());
            logln(a);
            clients[client_id]->write(response_buffer);

            logln("Connection closed!");
            loop.remove_read(clients[client_id]->fd());
        });

        // TODO: Make recv/reading bytes part of server/socket interface
        /*
        auto sock = server->accept();

        sock.on_ready_to_read = [&sock]() {
            auto bytes = sock.read_all();
            std::string_view message((char const*)bytes.data(), rc);
            std::cout << fd << ": " << message;
        }
        */
    };

    signal(SIGINT, [](int) {
        printf("Received Ctrl+C, shutting down.\n");
        exit(EXIT_SUCCESS);
    });

    loop.add_read(fileno(stdin), [](EventLoop const&) {
        printf("Received Enter press, shutting down.");
        exit(EXIT_SUCCESS);
    });

    loop.exec();

    return EXIT_SUCCESS;
}