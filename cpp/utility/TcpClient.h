// Copyright (c) 2024 - Tim Blackstone

#pragma once

#include "Socket.h"
#include <functional>
#include <optional>

class Bytes;
class EventLoop;
class Ipv4Address;

class TcpClient : public BufferedSocket {
public:
    static std::optional<TcpClient> create();

    TcpClient(TcpClient const&) = delete;
    TcpClient operator=(TcpClient const&) = delete;

    TcpClient(int socket_fd);

    bool connect(Ipv4Address host, int port);

    std::function<void(EventLoop&, Bytes&&)> on_read;

private:
};
