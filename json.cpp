#include "http/JsonObject.hpp"
#include "utility/String.hpp"
#include "utility/log.hpp"
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>

// https://www.json.org/json-en.html
// https://en.cppreference.com/w/cpp/utility/variant/visit
// https://en.cppreference.com/w/cpp/utility/initializer_list

enum class JsonState {
    Start,
    Key,
    Value,
    Malformed,
    Complete
};

std::optional<std::unordered_map<std::string, std::variant<double, std::string, std::vector<double>>>>
parse_json(std::string message)
{
    std::unordered_map<std::string, std::variant<double, std::string, std::vector<double>>> message_json = {};
    auto it = message.begin();

    auto consume = [&it, &message](char c) -> bool {
        if (it == message.end())
            return false;

        // printf("trying to consume %c as %c\n", (*it), c);

        bool consumable = (*it) == c;
        if (consumable) {
            // logln("Consumed {}", std::string { c });
            ++it;
        }
        return consumable;
    };

    auto consume_until = [&it, &message](char c) -> std::string {
        if (it == message.end())
            return "";

        std::string val;
        while ((*it) != c) {
            val += (*it);
            ++it;
        }
        return val;
    };

    auto discard_until = [&it, &message](char c) -> void {
        while (it != message.end() && *it != c)
            ++it;
    };

    auto make_string = [&it, &message]() -> std::string {
        if (it == message.end())
            return "";

        std::string val;
        bool escape = false;
        while ((*it) != '"' || escape) {
            if ((*it) == '\\' && !escape)
                escape = true;
            else if (!escape)
                val += (*it);
            else {
                val += '\\' + (*it);
                escape = false;
            }

            ++it;
        }
        return val;
    };

    auto make_int = [&it]() -> std::optional<double> {
        std::string val;
        while ((*it) == '-' || ((*it) >= '0' && (*it) <= '9') || (*it) == '.') {
            val += (*it);
            ++it;
        }
        return atof(val.c_str());
    };

    auto state = JsonState::Start;
    std::string key;
    while (it != message.end() && state != JsonState::Malformed && state != JsonState::Complete) {
        switch (state) {
        case JsonState::Start:
            if (consume('{')) {
                state = JsonState::Key;
            } else {
                state = JsonState::Malformed;
            }

            break;
        case JsonState::Key:
            if (consume('}')) {
                state = JsonState::Complete;
                break;
            }

            if (!consume('"')) {
                state = JsonState::Malformed;
                break;
            }
            key = make_string();
            logln("{} key", key);
            if (!consume('"')) {
                state = JsonState::Malformed;
                break;
            }
            if (!consume(':')) {
                state = JsonState::Malformed;
                break;
            }

            state = JsonState::Value;

            break;

        case JsonState::Value:
            if (consume('"')) {
                logln("parsing string - unhandled");
                discard_until('"'); // strings
                consume('"');
            } else if (consume('{')) {
                logln("parsing object - unhandled");
                discard_until('}'); // objects
                consume('}');
            } else if (consume('[')) {
                logln("parsing array");
                auto csv = consume_until(']');
                auto values_view = split_view(csv, ',');

                std::vector<double> json_array;
                for (auto value : values_view) {
                    json_array.push_back(std::stod(std::string { value }));
                }

                message_json.insert({ key, json_array });
                consume(']'); // arrays
            } else if ((*it) == '-' || ((*it) >= '0' && (*it) <= '9')) {
                logln("parsing number");
                message_json.insert({ key, make_int().value() }); // numbers
            } else if ((*it) == 't' || (*it) == 'f') {
                logln("parsing bool");
                while ((*it) != ',' && (*it) != '}')
                    ++it; // bools
            } else {
                printf("Failed at %c\n", *it);
                state = JsonState::Malformed;
                break;
            }

            if (consume(',')) {
                logln("reading comma");
                state = JsonState::Key;
            } else if (consume('}')) {

                logln("reading final brace");
                state = JsonState::Complete;
            } else {
                logln("broken");
                state = JsonState::Malformed;
            }

            break;
        case JsonState::Malformed:
            printf("Failed at '%c'\n", *it);
            break;
        }
    }

    return message_json;
}

int main()
{
    auto messageOne = "{\"number\":12,\"numberb\":4.3,\"test\":{\"a\":\"b\"},\"Values\":[50,100,100,100,100,100,100,100,50]}";
    auto jsonOne = parse_json(messageOne);
    auto messageTwo = "{\"entryone\":4,\"entrytwo\":12,\"Values\":[50,100,100,100,100,100,100,100,50]}";
    auto jsonTwo = parse_json(messageTwo);

    if (!jsonOne.has_value())
        return 1;
    if (!jsonTwo.has_value())
        return 1;

    logln("Parsing {}", messageOne);
    for (auto& entry : *jsonOne) {
        if (std::holds_alternative<double>(entry.second))
            logln("{}: {}", entry.first, std::get<double>(entry.second));
        else if (std::holds_alternative<std::string>(entry.second))
            logln("{}: {}", entry.first, std::get<std::string>(entry.second));
        else if (std::holds_alternative<std::vector<double>>(entry.second))
            logln("{}: {}", entry.first, std::get<std::vector<double>>(entry.second));
        else
            logln("{}: unhandled", entry.first);
    }

    logln("Parsing {}", messageTwo);
    for (auto& entry : *jsonTwo) {
        if (std::holds_alternative<double>(entry.second))
            logln("{}: {}", entry.first, std::get<double>(entry.second));

        if (std::holds_alternative<std::string>(entry.second))
            logln("{}: {}", entry.first, std::get<std::string>(entry.second));

        if (std::holds_alternative<std::vector<double>>(entry.second))
            logln("{}: {}", entry.first, std::get<std::vector<double>>(entry.second));
    }
}