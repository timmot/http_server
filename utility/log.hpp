#pragma once

#include <assert.h>
#include <concepts>
#include <cxxabi.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string>
#include <string_view>
#include <typeinfo>
#include <vector>

// https://mpark.github.io/programming/2017/05/26/constexpr-function-parameters/

// Apologies SerenityOS

namespace HiddenLog {

template <typename... Args>
void compiletime_fail(Args...);

template <typename>
inline constexpr bool always_false = false;

template <size_t N>
consteval auto count_format_braces(char const (&format_string)[N])
{
    bool found = false;
    size_t count = 0;
    for (size_t i = 0; i < N; ++i) {
        if (format_string[i] == '{')
            found = true;
        else if (format_string[i] == '}' && found)
            count++;
        else
            found = false;
    }

    return count;
}

template <size_t N, size_t params>
consteval auto verify_format_string(char const (&format_string)[N])
{
    if (count_format_braces(format_string) != params)
        compiletime_fail("Number of braces doesn't much number of parameters");
}

template <typename... Args>
struct CheckedFormatString {
    template <size_t N>
    consteval CheckedFormatString(char const (&format_string)[N])
        : m_string { format_string, N - 1 }
    {
        verify_format_string<N, sizeof...(Args)>(format_string);
    }

    CheckedFormatString(std::string const& format_string)
        : m_string { format_string }
    {
    }

    std::string_view m_string;
};

// Couldn't import log.hpp more than once?
// Having these not inline broke the one definition rule (ODR)
// https://clang.llvm.org/extra/clang-tidy/checks/misc/definitions-in-headers.html
// https://en.cppreference.com/w/cpp/language/definition
inline std::string stringify(char const* string) { return string; }
inline std::string stringify(char* string) { return string; }
inline std::string stringify(std::string_view string) { return std::string(string); }
inline std::string stringify(std::string const& string) { return string; }

template <std::integral I>
std::string stringify(I number) { return std::to_string(number); }

template <std::floating_point F>
std::string stringify(F number)
{
    std::stringstream buffer;
    buffer << std::fixed << std::setprecision(2) << number;
    return buffer.str();
}

template <typename T>
std::string stringify(std::vector<T> const& vector)
{
    std::string out;
    for (size_t i = 0; i < vector.size(); ++i) {
        out += stringify(vector[i]);
        if (i < vector.size() - 1)
            out += ",";
    }

    return out;
}

template <typename T>
std::string stringify(T)
{
    static_assert(always_false<T>, "Failed to format type");

    // Runtime failure
    /*auto name = abi::__cxa_demangle(typeid(anything).name(), nullptr, nullptr, nullptr);
    printf("Failed to format %s\n", name);
    assert(0);*/
    return {};
}

template <class... Args>
std::string format(std::string_view format_string, Args... arg)
{
    auto output = std::string(format_string);

    if constexpr (sizeof...(Args) > 0) {
        auto last_position = 0;

        ([&] {
            auto template_position = output.find("{}", last_position);
            last_position = template_position;
            output.replace(template_position, 2, stringify(arg));
        }(),
            ...);
    }

    return output;
}
}

template <typename... Args>
using CheckedFormatString = HiddenLog::CheckedFormatString<std::type_identity_t<Args>...>;

template <typename... Args>
void logln(CheckedFormatString<Args...>&& format_string, Args... args)
{
    puts(HiddenLog::format(format_string.m_string, args...).c_str());
}