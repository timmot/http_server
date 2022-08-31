// Copyright (c) 2022 - Tim Blackstone

#pragma once

#include <functional>
#include <optional>
#include <string_view>

class EventLoop;

class TcpServer {
public:
    static std::optional<TcpServer> create();
    ~TcpServer();
    bool listen(std::string_view host, uint16_t port);
    int accept();

    TcpServer(TcpServer const&) = delete;
    TcpServer operator=(TcpServer const&) = delete;

    TcpServer(int socket_fd)
        : m_socket_fd(socket_fd)
    {
    }

    std::function<void(EventLoop&)> on_read;

private:
    void sendbuftosck(int sckfd, const char* buf, int len);

    int m_socket_fd;
};