// Copyright (c) 2022 - Tim Blackstone
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

class Bytes {
public:
    Bytes(Bytes const& bytes)
        : m_size(bytes.m_size)
    {
        m_data = (uint8_t*)malloc(m_size);
        memcpy(m_data, bytes.data(), bytes.size());
    }

    Bytes const& operator=(Bytes const& bytes)
    {
        m_size = bytes.size();
        m_data = (uint8_t*)malloc(m_size);
        memcpy(m_data, bytes.data(), bytes.size());
        return *this;
    }

    Bytes(void const* bytes, size_t size)
    {
        m_data = (uint8_t*)malloc(size);
        memcpy(m_data, bytes, size);
        m_size = size;
    }

    Bytes(size_t size)
    {
        m_data = (uint8_t*)malloc(size);
        m_size = size;
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
    void trim(size_t new_size)
    {
        if (new_size < m_size)
            m_size = new_size;
    }

private:
    uint8_t* m_data;
    size_t m_size { 0 };
};