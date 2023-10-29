// Copyright (c) 2022 - Tim Blackstone

#pragma once

#include "JsonValue.hpp"
#include <vector>

class JsonArray {
public:
    JsonArray()
    {
    }
    /*
        void debug()
        {
            puts("[\n");
            for (auto& entry : m_array) {
                logln("\"{}\": {},", entry.debug());
            }
            puts("]\n");
        }*/

private:
    std::vector<JsonValue> m_array;
};
