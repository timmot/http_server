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

    std::optional<Bytes> read(Bytes& buffer)
    {
        if (m_eof && m_used_size == 0)
            return {};

        if (m_used_size == 0)
            populate_buffer();

        auto readable_size = std::min(m_used_size, buffer.size());
        printf("readable:%ld used:%ld\n", readable_size, m_used_size);
        auto buffer_to_take = m_buffer.slice(0, readable_size);
        auto buffer_to_shift = m_buffer.slice(readable_size, m_used_size);
        m_buffer.overwrite(0, buffer_to_shift.data(), m_used_size);
        m_used_size -= readable_size;

        printf("remaining in buffer:%ld\n", m_used_size);

        // Copy constructor memcpy's the actual data (not just the pointer ref)
        return buffer_to_take;
    }

    std::optional<Bytes> read_line()
    {
        // NOTE: breaks if its the first character
        return read_until('\n');
    }

    std::optional<Bytes> read_until(char candidate)
    {
        if (m_eof && m_used_size == 0)
            return {};

        if (m_used_size == 0)
            populate_buffer();

        if (m_used_size > 0) {
            auto maybe_index = m_buffer.find((uint8_t)candidate, m_used_size);
            if (maybe_index.has_value()) {
                if (*maybe_index == 0)
                    return Bytes {};

                auto buffer_to_take = m_buffer.slice(0, *maybe_index);
                auto buffer_to_move = m_buffer.slice(*maybe_index, m_used_size);
                m_buffer.overwrite(0, buffer_to_move.data(), m_used_size);
                m_used_size -= *maybe_index;

                return buffer_to_take;
            } else {
                // Return as a line even if there's no newline
                if (m_used_size > 0) {
                    auto buffered_size = m_used_size;
                    m_used_size = 0;

                    return m_buffer.slice(0, buffered_size);
                }
            }
        }

        return Bytes {};
    }

    // TODO: Read exactly the amount of bytes requests or fail and leave any remaining bytes in the m_buffer
    Bytes read_exactly(size_t n)
    {
        while (m_used_size < n) {
            printf("%ld\n", m_used_size);
            populate_buffer();
        }

        auto readable_size = std::min(m_used_size, n);
        auto buffer_to_take = m_buffer.slice(0, readable_size);

        auto buffer_to_shift = m_buffer.slice(readable_size, m_used_size);
        m_buffer.overwrite(0, buffer_to_shift.data(), m_used_size);
        m_used_size -= readable_size;

        return buffer_to_take;
    }

    int fd() const
    {
        return m_socket->fd();
    }

    ssize_t write(const Bytes& buffer)
    {
        return m_socket->write(buffer);
    }

private:
    void populate_buffer()
    {
        ssize_t read_count;
        do {
            // TODO: Ensure buffer size has enough room to accept this subspan
            m_buffer.ensure(m_used_size + 1024);

            auto buffer = m_buffer.span().subspan(m_used_size, 1024);
            read_count = m_socket->read(buffer);
            printf("read %ld\n", read_count);

            if (read_count == -1) {
                printf("hit error\n");
                break;
            }

            if (read_count == 0) {
                printf("hit eof\n");
                m_eof = true;
                break;
            }

            // add buffer to m_buffer
            if (read_count > 0) {
                printf("populating %ld\n", read_count);
                m_used_size += read_count;
            }
        } while (read_count > 0);
    }

    Bytes m_buffer;
    size_t m_used_size;
    bool m_eof;
    std::unique_ptr<Socket> m_socket;
};
