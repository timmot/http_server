// Copyright (c) 2022 - Tim Blackstone

#include "Socket.h"
#include "Bytes.hpp"
#include <cstdio>
#include <sys/socket.h>
#include <unistd.h>

std::unique_ptr<Socket> Socket::create_from_fd(int fd)
{
    return std::make_unique<Socket>(fd);
}

Socket::~Socket()
{
    close(m_fd);
}

ssize_t Socket::read(Bytes& buffer)
{
    return recv(m_fd, (void*)buffer.data(), buffer.size(), 0);
}

ssize_t Socket::read(std::span<uint8_t> buffer)
{
    return recv(m_fd, (void*)buffer.data(), buffer.size(), 0);
}

ssize_t Socket::write(Bytes const& buffer)
{
    ssize_t sent = 0;
    while (sent < (ssize_t)buffer.size()) {
        int write_count = send(m_fd, (void*)(buffer.data() + sent), buffer.size() - sent, MSG_NOSIGNAL);

        if (write_count == -1)
            return -1;

        sent += write_count;
    }

    return sent;
}