// Copyright (c) 2022 - Tim Blackstone

#pragma once
#include <cassert>
#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

class JsonNull {
public:
    JsonNull() = default;
};

class JsonValue;
using JsonArray = std::vector<JsonValue>;
using JsonObject = std::unordered_map<std::string, JsonValue>;

class JsonValue {
public:
    using Value = std::variant<double, std::string, bool, JsonObject, JsonArray, JsonNull>;

    JsonValue()
        : m_value(JsonObject {})
    {
    }

    JsonValue(double value)
        : m_value(value)
    {
    }

    JsonValue(int value)
        : m_value(static_cast<double>(value))
    {
    }

    JsonValue(char const* value)
        : m_value(std::string { value })
    {
    }

    JsonValue(std::string value)
        : m_value(value)
    {
    }

    JsonValue(bool value)
        : m_value(value)
    {
    }

    JsonValue(JsonObject value)
        : m_value(value)
    {
    }

    JsonValue(JsonArray value)
        : m_value(value)
    {
    }

    JsonValue(JsonNull value)
        : m_value(value)
    {
    }

    std::string debug()
    {
        std::stringstream buffer;
        if (std::holds_alternative<double>(m_value)) {
            buffer << std::fixed << std::setprecision(2) << as_double();
        } else if (std::holds_alternative<std::string>(m_value)) {
            buffer << '"' << as_string() << '"';
        } else if (std::holds_alternative<bool>(m_value)) {
            buffer << (as_bool() ? "true" : "false");
        } else if (std::holds_alternative<JsonObject>(m_value)) {
            auto object = as_object();
            buffer << "{";
            for (auto key_pair : object)
                buffer << '"' << key_pair.first << "\": " << key_pair.second.debug() << ',';
            buffer << "}";
        } else if (std::holds_alternative<JsonArray>(m_value)) {
            auto array = as_array();
            buffer << "[";
            for (auto value : array)
                buffer << value.debug() << ',';
            buffer << "]";
        } else if (std::holds_alternative<JsonNull>(m_value)) {
            buffer << "null";
        }

        return buffer.str();
    }

    double as_double() const
    {
        assert(std::holds_alternative<double>(m_value));
        return std::get<double>(m_value);
    }

    std::string const& as_string() const
    {
        assert(std::holds_alternative<std::string>(m_value));
        return std::get<std::string>(m_value);
    }

    bool as_bool() const
    {
        assert(std::holds_alternative<bool>(m_value));
        return std::get<bool>(m_value);
    }

    JsonObject const& as_object() const
    {
        assert(std::holds_alternative<JsonObject>(m_value));
        return std::get<JsonObject>(m_value);
    }

    JsonArray const& as_array() const
    {
        assert(std::holds_alternative<JsonArray>(m_value));
        return std::get<JsonArray>(m_value);
    }

    bool is_null() const
    {
        return std::holds_alternative<JsonNull>(m_value);
    }

    bool is_double() const
    {
        return std::holds_alternative<double>(m_value);
    }

    bool is_string() const
    {
        return std::holds_alternative<std::string>(m_value);
    }

    bool is_bool() const
    {
        return std::holds_alternative<bool>(m_value);
    }

    bool is_object() const
    {
        return std::holds_alternative<JsonObject>(m_value);
    }

    bool is_array() const
    {
        return std::holds_alternative<JsonArray>(m_value);
    }

private:
    Value m_value;
};
