// Copyright (c) 2022 - Tim Blackstone

#pragma once

#include <memory>
#include <optional>

class Bytes;

class Socket {
public:
    static std::unique_ptr<Socket> create_from_fd(int fd);

    Socket(int fd)
        : m_fd(fd)
    {
    }

    ~Socket();

    ssize_t read(Bytes const& buffer);
    ssize_t write(Bytes const& buffer);

    int fd() const
    {
        return m_fd;
    }

private:
    int m_fd;
};