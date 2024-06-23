// Copyright (c) 2024 - Tim Blackstone

#pragma once

#include <chrono>

class Clock {
public:
    Clock()
        : m_time_point(std::chrono::high_resolution_clock::now())
    {
    }

    void reset()
    {
        m_time_point = std::chrono::high_resolution_clock::now();
    }

    int64_t s_and_reset()
    {
        auto value = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - m_time_point).count();
        reset();
        return value;
    }

    int64_t ms_and_reset()
    {
        auto value = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_time_point).count();
        reset();
        return value;
    }

    int64_t ns_and_reset()
    {
        auto value = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_time_point).count();
        reset();
        return value;
    }

    int64_t s()
    {
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - m_time_point).count();
    }

    int64_t ms()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_time_point).count();
    }

    int64_t ns()
    {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_time_point).count();
    }

private:
    std::chrono::system_clock::time_point m_time_point;
};
