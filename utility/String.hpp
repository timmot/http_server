// Copyright (c) 2022 - Tim Blackstone
#pragma once

#include <string>
#include <string_view>
#include <vector>

std::vector<std::string> split(std::string const& text, char separator)
{
    std::vector<std::string> output;

    if (text.find(separator) == std::string::npos)
        return { text };

    for (size_t i = 0; i < text.size();) {
        auto j = text.find_first_of(separator, i);
        if (j == std::string::npos) {
            // FIXME: if we get here on the first iteration, we get a new substring from 0 to n - 1
            j = text.size() - 1;
            output.push_back(text.substr(i, j - i));
            break;
        }
        output.push_back(text.substr(i, j - i));
        i = j + 1;
    }

    return output;
}

std::vector<std::string_view> split_view(std::string_view text, char separator)
{
    std::vector<std::string_view> output;

    if (text.find(separator) == std::string::npos)
        return { text };

    for (size_t i = 0; i < text.size();) {
        auto j = text.find_first_of(separator, i);
        if (j == std::string::npos) {
            j = text.size() - 1;
            auto view = text.substr(i, j - i);
            output.push_back(view);
            break;
        }
        auto view = text.substr(i, j - i);
        output.push_back(view);
        i = j + 1;
    }

    return output;
}