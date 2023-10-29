// Copyright (c) 2022 - Tim Blackstone

#pragma once

#include "JsonArray.hpp"
#include "JsonValue.hpp"
#include <string>
#include <unordered_map>
#include <vector>

class JsonObject {
public:
    JsonObject()
    {
    }

    JsonArray values() const;
    std::vector<std::string_view> keys() const;

    JsonValue& operator[](std::string const& key)
    {
        // assert(m_object.contains(key));
        return m_object.at(key);
    }
    /*
        void debug()
        {
            puts("{\n");
            for (auto& entry : m_object) {
                logln("\"{}\": {},", entry.first, entry.second.debug());
            }
            puts("}\n");
        }*/

private:
    std::unordered_map<std::string, JsonValue> m_object;
};
