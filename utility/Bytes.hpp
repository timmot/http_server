// Copyright (c) 2022 - Tim Blackstone

#pragma once

#include <assert.h>
#include <optional>
#include <span>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>

/*
how does my Bytes compare to a Span and a ByteBuffer?

a span just stores any T in a C array with a size
vs an array, a span is considered non-owning

consider buffer as owning as well?

*/

class Bytes {
public:
    // TODO: implement move constructor
    // NOTE: move = default; only works if all values can be moved, this generally means no raw pointers
    // raw pointers must have their address copied
    Bytes(Bytes const& bytes)
        : m_size(bytes.m_size)
    {
        m_data = static_cast<uint8_t*>(malloc(m_size));
        memcpy(m_data, bytes.data(), bytes.size());
    }

    Bytes const& operator=(Bytes const& bytes)
    {
        m_size = bytes.size();
        m_data = static_cast<uint8_t*>(malloc(m_size));
        memcpy(m_data, bytes.data(), bytes.size());
        return *this;
    }

    Bytes(void const* bytes, size_t size)
        : m_size(size)
    {
        m_data = static_cast<uint8_t*>(malloc(size));
        memcpy(m_data, bytes, size);
    }

    Bytes(std::string const& string)
        : m_size(string.size())
    {
        m_data = static_cast<uint8_t*>(malloc(m_size));
        memcpy(m_data, string.data(), string.size());
    }

    Bytes(size_t size)
    {
        m_data = (uint8_t*)malloc(size);
        m_size = size;
    }

    Bytes()
    {
        m_data = nullptr;
        m_size = 0;
    }

    ~Bytes()
    {
        free(m_data);
        m_size = 0;
    }

    uint8_t& operator[](size_t i) { return data()[i]; }

    uint8_t* data() { return m_data; }
    uint8_t const* data() const { return m_data; }
    size_t size() const { return m_size; }
    std::span<uint8_t> span()
    {
        return { data(), size() };
    }

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

    uint32_t to_uint32()
    {
        return static_cast<uint32_t>(m_data[0] << 24 | m_data[1] << 16 | m_data[2] << 8 | m_data[3]);
    }

    int32_t to_int32()
    {
        return static_cast<int32_t>(m_data[0] << 24 | m_data[1] << 16 | m_data[2] << 8 | m_data[3]);
    }

    uint16_t to_uint16()
    {
        return static_cast<uint16_t>(m_data[0] << 8 | m_data[1]);
    }

    int16_t to_int16()
    {
        return static_cast<int16_t>(m_data[0] << 8 | m_data[1]);
    }

private:
    uint8_t* m_data;
    size_t m_size { 0 };
};
