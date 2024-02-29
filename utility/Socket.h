// Copyright (c) 2022 - Tim Blackstone

#pragma once

#include "Bytes.hpp"
#include <algorithm>
#include <memory>
#include <optional>
#include <stdio.h>

class Socket {
public:
    static std::unique_ptr<Socket> create_from_fd(int fd);

    Socket(int fd)
        : m_fd(fd)
    {
    }

    Socket& operator=(Socket const& socket) = default;

    ~Socket();

    ssize_t read(Bytes& buffer);
    ssize_t read(std::span<uint8_t> buffer);
    ssize_t write(Bytes const& buffer);

    int fd() const
    {
        return m_fd;
    }

private:
    int m_fd;
};

class BufferedSocket {
public:
    BufferedSocket(int fd)
        : m_buffer(Bytes(65536))
        , m_used_size(0)
        , m_eof(false)
    {
        m_socket = Socket::create_from_fd(fd);
    }

    BufferedSocket(std::unique_ptr<Socket> socket)
        : m_buffer(Bytes(65536))
        , m_used_size(0)
        , m_eof(false)
        , m_socket(std::move(socket))
    {
    }

    BufferedSocket(BufferedSocket&& other) = default;
    BufferedSocket& operator=(BufferedSocket&& other) = default;

    // NOTE: The default copy constructor's break our unique_ptr member
    BufferedSocket(BufferedSocket const&) = delete;
    BufferedSocket& operator=(BufferedSocket const&) = delete;

    std::optional<Bytes> read(Bytes& buffer);
    std::optional<Bytes> read_line();
    std::optional<Bytes> read_until(char candidate);

    // TODO: Read exactly the amount of bytes requests or fail and leave any remaining bytes in the m_buffer
    Bytes read_exactly(size_t n);

    int fd() const
    {
        return m_socket->fd();
    }

    ssize_t write(const Bytes& buffer)
    {
        return m_socket->write(buffer);
    }

private:
    void populate_buffer();

    Bytes m_buffer;
    size_t m_used_size;
    bool m_eof;
    std::unique_ptr<Socket> m_socket;
};
