#include <assert.h>
#include <concepts>
#include <iomanip>
#include <iostream>
#include <stdio.h>
#include <string>

namespace HiddenLog {
std::string stringify(char const* string) { return string; }
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
    printf("Failed to format");
    assert(0);
    return {};
}

template <class... Args>
std::string format(std::string const& format_string, Args... arg)
{
    auto output = format_string;
    auto last_position = 0;

    ([&] {
        auto template_position = output.find("{}", last_position);
        last_position = template_position;
        output.replace(template_position, 2, stringify(arg));
    }(),
        ...);

    return output;
}
}

template <class... Args>
inline void outln(std::string const& format_string, Args... args)
{
    std::cout << HiddenLog::format(format_string, args...) << '\n';
}