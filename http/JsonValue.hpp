// Copyright (c) 2022 - Tim Blackstone

#pragma once

#include "JsonArray.hpp"
#include "JsonObject.hpp"
#include <string>
#include <variant>

class JsonNull {
public:
    JsonNull() = default;
};

/*
template <class>
inline constexpr bool always_false_v = false;*/

class JsonValue {
public:
    JsonValue()
    {
    }
    /*
        std::string debug()
        {
            std::stringstream buffer;

            std::visit([&buffer](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, double>)
                    buffer << std::fixed << std::setprecision(2) << arg;
                else if constexpr (std::is_same_v<T, std::string>)
                    buffer << arg;
                else if constexpr (std::is_same_v<T, bool>)
                    buffer << (arg ? "true" : "false");
                else if constexpr (std::is_same_v<T, JsonObject>)
                    buffer << "object";
                else if constexpr (std::is_same_v<T, JsonArray>)
                    buffer << "array";
                else if constexpr (std::is_same_v<T, JsonNull>)
                    buffer << "null";
                else
                    static_assert(always_false_v<T>, "non-exhaustive visitor!");
            },
                m_value);

            return buffer.str();
        }*/

    double to_double() const;
    std::string const& to_string() const;
    bool to_bool() const;
    JsonObject const& to_object() const;
    JsonArray const& to_array() const;
    bool is_null() const;

private:
    std::variant<double, std::string, bool, JsonObject, JsonArray, JsonNull> m_value;
};
