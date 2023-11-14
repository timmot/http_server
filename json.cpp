#include "http/JsonValue.hpp"
#include "utility/String.hpp"
#include "utility/log.hpp"
#include <cassert>
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

std::optional<JsonValue> parse_json(std::string message)
{
    JsonObject message_json = {};
    auto it = message.begin();

    auto peek = [&it, &message](char c) -> bool {
        if (it == message.end())
            return false;

        return (*it) == c;
    };

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

    auto consume_after = [&it, &message](char c) -> std::string {
        if (it == message.end())
            return "";

        std::string val;
        while ((*it) != c) {
            val += (*it);
            ++it;
        }
        val += (*it);
        ++it;
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
                logln("parsing string");
                message_json.insert({ key, consume_until('"') });
                consume('"');
            } else if (peek('{')) {
                logln("parsing object");
                auto object_text = consume_after('}');
                auto object = parse_json(object_text);
                if (object.has_value())
                    message_json.insert({ key, *object });
                else {
                    state = JsonState::Malformed;
                    break;
                }
            } else if (consume('[')) {
                logln("parsing array");
                auto csv = consume_until(']');
                auto values_view = split_view(csv, ',');

                JsonArray json_array;
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
                message_json.insert({ key, (*it) == 't' });
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
        case JsonState::Complete:
            // leave the loop
            break;
        }
    }

    return { message_json };
}

int main()
{
    auto messageOne = "{\"number\":12,\"numberb\":4.3,\"test\":{\"a\":\"b\"},\"Values\":[50,100,100,100,100,100,100,100,50],\"hello\":\"world\",\"other\":false}";
    auto jsonOne = parse_json(messageOne);

    logln("Parsing {}", messageOne);
    assert(jsonOne.has_value());
    assert(jsonOne->is_object());
    logln("Result: {}", jsonOne->debug());

    auto jsonObject = jsonOne->as_object();
    assert(jsonObject.contains("number"));
    assert(jsonObject.contains("numberb"));
    assert(jsonObject.contains("test"));
    assert(jsonObject.contains("Values"));
    assert(jsonObject.contains("hello"));
    assert(jsonObject.contains("other"));

    assert(jsonObject["number"].is_double());
    assert(jsonObject["numberb"].is_double());
    assert(jsonObject["test"].is_object());
    assert(jsonObject["Values"].is_array());
    assert(jsonObject["hello"].is_string());
    assert(jsonObject["other"].is_bool());
}
