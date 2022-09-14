#include "utility/Bytes.hpp"
#include "utility/EventLoop.h"
#include "utility/Socket.h"
#include "utility/TcpServer.h"
#include "utility/log.hpp"
#include <iostream>
#include <memory>
#include <vector>

int main()
{
    auto server = TcpServer::create();

    EventLoop loop;

    server->listen("0.0.0.0", 10000);
    if (server.has_value())
        logln("Listening on port 10000");
    else
        return 1;

    std::vector<std::unique_ptr<Socket>> clients;
    server->on_read = [&server, &clients](EventLoop& loop) {
        auto client = clients.emplace_back(server->accept()).get();
        logln("Accepted new client {}", client->fd());

        loop.add_read(client->fd(), [client, &clients](EventLoop& loop) {
            Bytes buf(1024);
            auto nread = client->read(buf);

            if (nread <= 0) {
                logln("Connection closed for {}, {} clients remaining", client->fd(), clients.size());
                loop.remove_read(client->fd());
                clients.erase(std::remove_if(clients.begin(), clients.end(), [client](auto& client_ptr) { return client_ptr->fd() == client->fd(); }));
            }

            Bytes new_buf((char*)buf.data(), nread);
            client->write(new_buf);
        });
    };

    loop.exec();

    return 0;
}
