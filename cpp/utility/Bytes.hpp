// Copyright (c) 2022 - Tim Blackstone

#pragma once

#include <assert.h>
#include <optional>
#include <span>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string_view>
#include <unistd.h>

class Bytes {
public:
    template <std::size_t N>
    Bytes(const char (&chars)[N])
    {
        m_data = new (std::nothrow) uint8_t[N];
        m_size = N;
        memcpy(m_data, &chars, N);
    }

    Bytes(void const* bytes, size_t size)
        : m_size(size)
    {
        m_data = new (std::nothrow) uint8_t[size];
        memcpy(m_data, bytes, size);
    }

    explicit Bytes(std::string_view string)
        : m_size(string.size())
    {
        m_data = new (std::nothrow) uint8_t[m_size];
        memcpy(m_data, string.data(), string.size());
    }

    Bytes(size_t size)
    {
        m_data = new (std::nothrow) uint8_t[size];
        m_size = size;
    }

    Bytes()
        : m_data(nullptr)
        , m_size(0)
    {
    }

    Bytes(Bytes const& bytes)
        : m_size(bytes.m_size)
    {
        m_data = new (std::nothrow) uint8_t[m_size];
        memcpy(m_data, bytes.data(), bytes.size());
    }

    Bytes& operator=(Bytes const& bytes)
    {
        m_size = bytes.size();
        m_data = new (std::nothrow) uint8_t[m_size];
        memcpy(m_data, bytes.data(), bytes.size());
        return *this;
    }

    Bytes(Bytes&& other)
        : m_data(other.m_data)
        , m_size(std::move(other.m_size))
    {
        other.m_data = nullptr;
        other.m_size = 0;
    }

    Bytes& operator=(Bytes&& other)
    {
        delete[] m_data;
        m_size = std::move(other.m_size);
        other.m_size = 0;
        m_data = other.m_data;
        other.m_data = nullptr;

        return *this;
    }

    ~Bytes()
    {
        delete[] m_data;
        m_size = 0;
    }

    uint8_t& operator[](size_t i)
    {
        return data()[i];
    }

    uint8_t* data()
    {
        return m_data;
    }

    uint8_t const* data() const
    {
        return m_data;
    }

    size_t size() const
    {
        return m_size;
    }

    std::span<uint8_t> span()
    {
        return { data(), size() };
    }

    // TODO: this type feels wrong
    std::optional<size_t> find(uint8_t candidate, size_t length)
    {
        for (size_t i = 0; i < length; ++i)
            if (m_data[i] == candidate)
                return i + 1;

        return {};
    }

    Bytes slice(size_t start, size_t length)
    {
        assert(start + length <= m_size);
        return { m_data + start, length };
    }

    Bytes slice(size_t start)
    {
        assert(start <= m_size);
        return { m_data + start, m_size - start };
    }

    std::span<uint8_t> slice_span(size_t start, size_t length)
    {
        assert(start + length <= m_size);
        return span().subspan(start, length);
    }

    std::span<uint8_t> slice_span(size_t start)
    {
        assert(start <= m_size);
        return span().subspan(start, m_size - start);
    }

    void trim(size_t new_size)
    {
        if (new_size < m_size)
            m_size = new_size;
    }

    void overwrite(size_t start, uint8_t const* data, size_t length)
    {
        assert(start + length <= m_size);
        memcpy(m_data + start, data, length);
    }

    void overwrite(size_t start, Bytes const& other_bytes)
    {
        assert(start + other_bytes.size() <= m_size);
        memcpy(m_data + start, other_bytes.data(), other_bytes.size());
    }

    void zero()
    {
        memset(m_data, 0, m_size);
    }

    void debug()
    {
        printf("Address: %p ; size: %ld %02lX\n", (void*)m_data, m_size, m_size);
        for (size_t i = 0; i < size(); ++i) {
            if (m_data[i] >= 32 && m_data[i] <= 126)
                printf("0x%02X 0d%d\t%c\n", m_data[i], m_data[i], m_data[i]);
            else
                printf("0x%02X 0d%d\t[unprintable]\n", m_data[i], m_data[i]);
        }
    }

    void debug_hex()
    {
        printf("Address: %p ; size: %ld  %02lX\n", (void*)m_data, m_size, m_size);
        for (size_t i = 0; i < size(); ++i) {
            if (i != 0) {
                if (i % 16 == 0)
                    printf("\n0x%04zX: ", i);
                else
                    printf(" ");
            }

            printf("%02X", m_data[i]);
        }
        printf("\n");
    }

    void ensure(size_t size)
    {
        if (size < m_size)
            return;

        auto new_size = m_size;
        while (new_size < size)
            new_size *= 2;

        auto new_data = new (std::nothrow) uint8_t[new_size];
        memcpy(new_data, m_data, m_size);

        delete[] m_data;
        m_data = new_data;
        m_size = new_size;
    }

private:
    uint8_t* m_data;
    size_t m_size;
};
