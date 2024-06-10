// Copyright (c) 2023 - Tim Blackstone

#pragma once

#include <random>

class Random {
public:
    Random()
    {
        std::random_device random_device;
        m_random_engine = std::minstd_rand0(random_device());
    }

    template <std::floating_point T>
    T take_real()
    {
        std::uniform_real_distribution<T> m_uniform_distribution;
        return m_uniform_distribution(m_random_engine);
    }

    template <std::floating_point T>
    T take_real(T min, T max)
    {
        std::uniform_real_distribution<T> m_uniform_distribution(min, max);
        return m_uniform_distribution(m_random_engine);
    }

    template <std::integral T>
    T take_int()
    {
        std::uniform_int_distribution<T> m_uniform_distribution;
        return m_uniform_distribution(m_random_engine);
    }

    template <std::integral T>
    T take_int(T min, T max)
    {
        std::uniform_int_distribution<T> m_uniform_distribution(min, max);
        return m_uniform_distribution(m_random_engine);
    }

private:
    std::minstd_rand0 m_random_engine;
};
