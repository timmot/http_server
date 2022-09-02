#include "utility/TcpServer.h"
#include "utility/Bytes.hpp"
#include "utility/EventLoop.h"
#include "utility/Socket.h"
#include "utility/log.hpp"
#include <iostream>
#include <memory>
#include <signal.h>
#include <stdio.h>
#include <string_view>

int main(int argc, char* argv[])
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s host port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    auto server = TcpServer::create();

    if (!server.has_value())
        exit(EXIT_FAILURE);

    EventLoop loop;

    if (server->listen(argv[1], atoi(argv[2])))
        logln("listening on port: {}", argv[2]);

    std::vector<std::unique_ptr<Socket>> clients;
    server->on_read = [&server, &clients](EventLoop& loop) {
        clients.emplace_back(server->accept());
        auto client_id = clients.size() - 1;
        logln("Accepted new client {}", clients.back()->fd());

        loop.add_read(clients.back()->fd(), [&clients, client_id](EventLoop& loop) {
            Bytes buf(1024);
            auto nread = clients[client_id]->read(buf);

            if (nread <= 0) {
                logln("Connection closed! {}", nread);
                loop.remove_read(clients[client_id]->fd());
                return;
            }

            // Remove newline from message
            std::string_view message((char const*)buf.data(), nread - 1);
            logln("Client {}: {}", clients[client_id]->fd(), message);

            Bytes response("Thank you!", 10);
            clients[client_id]->write(response);

            logln("Connection closed!");
            loop.remove_read(clients[client_id]->fd());
        });

        // TODO: Make recv/reading bytes part of server/socket interface
        /*
        auto sock = server->accept();

        sock.on_ready_to_read = [&sock]() {
            auto bytes = sock.read_all();
            std::string_view message((char const*)bytes.data(), rc);
            std::cout << fd << ": " << message;
        }
        */
    };

    signal(SIGINT, [](int) {
        printf("Received Ctrl+C, shutting down.\n");
        exit(EXIT_SUCCESS);
    });

    loop.add_read(fileno(stdin), [](EventLoop const&) {
        printf("Received Enter press, shutting down.");
        exit(EXIT_SUCCESS);
    });

    loop.exec();

    return EXIT_SUCCESS;
}