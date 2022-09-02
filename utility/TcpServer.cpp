// Copyright (c) 2022 - Tim Blackstone

#include "TcpServer.h"
#include "Bytes.hpp"
#include "EventLoop.h"
#include "Socket.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFSIZE 1024

std::optional<TcpServer> TcpServer::create()
{
    int sock;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return {};
    // Set reusable
    int yes = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        return {};

    // Set non blocking
    int sock_opts = fcntl(sock, F_GETFL);
    fcntl(sock, F_SETFL, sock_opts | O_NONBLOCK);

    return { sock };
}

TcpServer::~TcpServer()
{
    close(m_socket_fd);
}

bool TcpServer::listen(std::string_view host, uint16_t port)
{
    sockaddr_in address = {};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;

    if (bind(m_socket_fd, (const sockaddr*)&address, sizeof address) == -1) {
        perror("server: bind");
        return false;
    }

    if (::listen(m_socket_fd, 10) == -1) {
        printf("couldn't listen");
        return false;
    }

    EventLoop::current().add_read(m_socket_fd, [this](EventLoop& loop) {
        if (on_read)
            on_read(loop);
    });

    return true;
}

std::unique_ptr<Socket> TcpServer::accept()
{
    return Socket::create_from_fd(::accept(m_socket_fd, NULL, NULL));
}