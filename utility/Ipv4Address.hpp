// Copyright (c) 2024 - Tim Blackstone
#pragma once

#include "log.hpp"
#include <arpa/inet.h>
#include <cstdio>
#include <netdb.h>
#include <optional>
#include <string_view>

class Ipv4Address {
public:
    constexpr Ipv4Address() = default;

    constexpr Ipv4Address(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
    {
        m_data = (d << 24) | (c << 16) | (b << 8) | a;
    }

    constexpr Ipv4Address(uint32_t network_order)
    {
        m_data = network_order;
    }

    static std::optional<Ipv4Address> from_string(std::string_view ip_address)
    {
        sockaddr_in sa;
        int rc = inet_pton(AF_INET, ip_address.data(), &(sa.sin_addr));
        if (rc <= 0) {
            if (rc == -1)
                perror("inet_pton");
            return {};
        }

        Ipv4Address ip;
        ip.m_data = sa.sin_addr.s_addr;
        return ip;
    }

    static std::optional<Ipv4Address> resolve_hostname(std::string_view host)
    {
        Ipv4Address ip;

        addrinfo hints = {};
        hints.ai_family = AF_INET;

        addrinfo* result;
        int s = getaddrinfo(host.data(), NULL, &hints, &result);
        if (s != 0) {
            fprintf(stderr, "%s", gai_strerror(s));
            return {};
        }

        if (result->ai_family == AF_INET) {
            auto psai = (sockaddr_in*)result->ai_addr;
            ip = psai->sin_addr.s_addr;
        }
        freeaddrinfo(result);

        return ip;
    }

    std::string to_string()
    {
        uint8_t a = m_data & 0xff;
        uint8_t b = (m_data >> 8) & 0xff;
        uint8_t c = (m_data >> 16) & 0xff;
        uint8_t d = (m_data >> 24) & 0xff;

        return format("{}.{}.{}.{}", a, b, c, d);
    }

    in_addr_t to_in_addr_t() const
    {
        return m_data;
    }

    constexpr bool operator==(Ipv4Address const& other) const = default;

private:
    // Stored in network order, so backwards
    uint32_t m_data {};
};
