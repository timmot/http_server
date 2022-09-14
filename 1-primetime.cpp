#include "utility/Bytes.hpp"
#include "utility/EventLoop.h"
#include "utility/Socket.h"
#include "utility/String.hpp"
#include "utility/TcpServer.h"
#include "utility/log.hpp"
#include <iostream>
#include <math.h>
#include <memory>
#include <string_view>
#include <vector>

struct Message {
    std::optional<std::string> method;
    std::optional<int> number;
};

enum class JsonState {
    Start,
    Key,
    Value,
    Malformed,
    Complete
};

std::optional<Message> parse_json(std::string message)
{
    Message message_json = {};
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
            if (key == "number") {
                // FIXME: what does 97717743314710839372219267221764680508953168505572670845853077 output?
                message_json.number = make_int();
            } else if (key == "method") {
                consume('"');
                message_json.method = make_string();
                consume('"');
            } else if (key == "nummar") {
                // FIXME: why doesn't this parse?
                return message_json;
            } else {
                if (consume('"')) {
                    discard_until('"'); // strings
                    consume('"');
                } else if (consume('{')) {
                    discard_until('}'); // objects
                    consume('}');
                } else if ((*it) == '-' || ((*it) >= '0' && (*it) <= '9')) {
                    make_int(); // numbers
                } else if ((*it) == 't' || (*it) == 'f') {
                    while ((*it) != ',' && (*it) != '}')
                        ++it; // bools
                } else {
                    printf("Failed at %c\n", *it);
                    state = JsonState::Malformed;
                    break;
                }
            }

            if (consume(',')) {
                state = JsonState::Key;
            } else if (consume('}')) {
                state = JsonState::Complete;
            } else {
                state = JsonState::Malformed;
            }

            break;
        case JsonState::Malformed:
            printf("Failed at '%c'\n", *it);
            break;
        }
    }

    if (message_json.method.has_value() && message_json.number.has_value() && *message_json.method == "isPrime" && state == JsonState::Complete) {
        return message_json;
    }

    logln("Failure to parse. method: {}, number: {}, state: {}", message_json.method.value_or("-"), message_json.number.value_or(-69), state == JsonState::Malformed ? "malformed" : "not-malformed");
    if (it != message.end())
        printf("Remaining message '%c'\n\t%s\n", (*it), std::string(it, message.end() - 1).c_str());

    return {};
}

static int largest_number = 0;
bool is_prime(int number)
{
    if (number > largest_number)
        largest_number = number;

    if (number <= 1)
        return false;

    for (int i = 2; i <= sqrt(number); i++)
        if (number % i == 0)
            return false;

    return true;
}

int main()
{
    auto server = TcpServer::create();

    EventLoop loop;

    server->listen("0.0.0.0", 10000);
    if (server.has_value())
        logln("Listening on port 10000");
    else
        return 1;

    static int count = 0;
    std::vector<std::unique_ptr<Socket>> clients;
    server->on_read = [&server, &clients](EventLoop& loop) {
        auto client = clients.emplace_back(server->accept()).get();
        logln("Accepted new client {}", client->fd());

        loop.add_read(client->fd(), [client, &clients](EventLoop& loop) {
            logln("starting read...");
            count = 0;
            Bytes buf(50'000);
            auto nread = client->read(buf);

            if (nread <= 0) {
                logln("Connection closed for {}, {} clients remaining", client->fd(), clients.size());
                loop.remove_read(client->fd());
                clients.erase(std::remove_if(clients.begin(), clients.end(), [client](auto& client_ptr) { return client_ptr->fd() == client->fd(); }));
                return;
            }

            logln("processing client request...");

            std::string client_request((char*)buf.data(), nread);
            // logln("{}==\n{}", client->fd(), client_request);
            auto messages = split(client_request, '\n');

            puts(client_request.c_str());
            for (auto message : messages) {
                ++count;

                auto maybe_json = parse_json(message);

                if (!maybe_json.has_value()) {
                    Bytes read_more(1024);
                    Bytes malformed("{}", 2);
                    client->write(malformed);
                    auto fd = client->fd();
                    loop.remove_read(client->fd());
                    clients.erase(std::remove_if(clients.begin(), clients.end(), [client](auto& client_ptr) { return client_ptr->fd() == client->fd(); }));

                    logln("Malformed. Connection closed");
                    // printf(">\t%s\n", message.c_str());
                    break;
                } else {
                    std::string response = "{\"method\":\"isPrime\",\"prime\":";
                    response += is_prime(*maybe_json->number) ? "true}\n" : "false}\n";

                    // logln("Responding with message {}, {} clients", response.substr(0, response.size() - 2), clients.size());

                    Bytes new_buf(response);
                    if (client->write(new_buf) == -1) {
                        logln("couldn't write: {}", count);
                        loop.remove_read(client->fd());
                        clients.erase(std::remove_if(clients.begin(), clients.end(), [client](auto& client_ptr) { return client_ptr->fd() == client->fd(); }));
                        break;
                    }
                }
            }
            logln("Returned all {} messages for request", count);
        });
    };

    loop.exec();

    logln("program ended happily?");

    return 0;
}
