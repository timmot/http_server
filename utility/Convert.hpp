// Copyright (c) 2024 - Tim Blackstone

#pragma once

#include <optional>
#include <stdexcept>
#include <string>

class Convert {
public:
    template <typename T>
    static std::optional<T> to(std::string const& s)
    try {
        if constexpr (std::is_same<T, int>()) {
            return std::stoi(s);
        } else if constexpr (std::is_same<T, double>()) {
            return std::stod(s);
        } else {
            return {};
        }
    } catch (std::invalid_argument const&) {
        return {};
    } catch (std::out_of_range const&) {
        return {};
    }
};
