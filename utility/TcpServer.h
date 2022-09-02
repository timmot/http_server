// Copyright (c) 2022 - Tim Blackstone

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string_view>

class EventLoop;
class Socket;

class TcpServer {
public:
    static std::optional<TcpServer> create();
    ~TcpServer();
    bool listen(std::string_view host, uint16_t port);
    std::unique_ptr<Socket> accept();

    TcpServer(TcpServer const&) = delete;
    TcpServer operator=(TcpServer const&) = delete;

    TcpServer(int socket_fd)
        : m_socket_fd(socket_fd)
    {
    }

    std::function<void(EventLoop&)> on_read;

private:
    int m_socket_fd;
};