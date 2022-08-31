#pragma once

#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

class Bytes
{
public:
    Bytes(char const* bytes, size_t size)
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

private:
    uint8_t* m_data;
    size_t m_size { 0 };
};