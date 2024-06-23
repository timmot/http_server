#include "../utility/Bytes.hpp"
#include "../utility/EventLoop.h"
#include "../utility/Socket.h"
#include "../utility/TcpServer.h"
#include "../utility/log.hpp"
#include <iostream>
#include <map>
#include <memory>
#include <vector>

uint32_t to_uint32(std::span<uint8_t> data)
{
    return static_cast<uint32_t>(data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3]);
}

int32_t to_int32(std::span<uint8_t> data)
{
    return static_cast<int32_t>(data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3]);
}

uint16_t to_uint16(std::span<uint8_t> data)
{
    return static_cast<uint16_t>(data[0] << 8 | data[1]);
}

int16_t to_int16(std::span<uint8_t> data)
{
    return static_cast<int16_t>(data[0] << 8 | data[1]);
}

class Client : public BufferedSocket {
public:
    std::map<int32_t, int32_t> database {};
};

int main()
{
    auto server = TcpServer::create();

    EventLoop loop;

    server->listen("0.0.0.0", 10000);
    if (server.has_value())
        logln("Listening on port 10000");
    else
        return 1;

    std::vector<Client> clients;
    server->on_read = [&server, &clients](EventLoop& loop) {
        clients.emplace_back(Client { server->accept() });
        auto& client = clients.back();
        logln("Accepted new client {}", client.fd());

        while (true) {
            auto buf = client.read_exactly(9);

            /*if (b <= 0) {
                logln("Connection closed for {}, {} clients remaining", client->fd(), clients.size());
                loop.remove_read(client->fd());
                clients.erase(std::remove_if(clients.begin(), clients.end(), [client](auto& client_ptr) { return client_ptr->fd() == client->fd(); }));
            }*/

            if (buf[0] == 'I') { // Insert
                auto timestamp = to_int32(buf.span().subspan(1, 4));
                auto price = to_int32(buf.span().subspan(5, 4));

                client.database.insert({ timestamp, price });
                printf("insert %d %d\n", timestamp, price);
            } else if (buf[0] == 'Q') { // Query
                auto min_time = to_int32(buf.span().subspan(1, 4));
                auto max_time = to_int32(buf.span().subspan(5, 4));

                int32_t mean = 0;
                int32_t count = 0;
                for (auto const& entry : client.database) {
                    if (entry.first >= min_time && entry.first <= max_time) {
                        mean += entry.second;
                        ++count;
                    }
                }
                mean /= count;

                printf("query %d %d\n", min_time, max_time);
                printf("result: %d %d\n", mean, count);

                mean = __builtin_bswap32(mean);

                Bytes result(&mean, 4);
                result.debug();
                client.write(result);
            } else {
                printf("brokey\n");
                buf.debug();
            }
        }
    };

    loop.exec();

    return 0;
}
