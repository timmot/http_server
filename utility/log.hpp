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

// Apologies SerenityOS

namespace HiddenLog {

template <typename... Args>
void compiletime_fail(Args...);

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

    std::string_view m_string;
};

std::string stringify(char const* string)
{
    return string;
}
std::string stringify(char* string) { return string; }
std::string stringify(std::string_view string) { return std::string(string); }
std::string stringify(std::string string) { return std::forward<std::string>(string); }

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
std::string stringify(T anything)
{
    auto name = abi::__cxa_demangle(typeid(anything).name(), nullptr, nullptr, nullptr);
    printf("Failed to format %s\n", name);

    assert(0);
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
    std::cout << HiddenLog::format(format_string.m_string, args...) << '\n';
}