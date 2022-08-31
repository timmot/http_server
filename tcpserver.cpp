#include "utility/TcpServer.h"
#include "utility/Bytes.hpp"
#include "utility/EventLoop.h"
#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <string_view>
#include <sys/socket.h>

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
        printf("listening on port: %s\n", argv[2]);

    // Notifiers add to a static thread-local hashtable stored in EventLoop
    // Each pump we wait for events and loop through all notifiers and add them to the queue
    // And after the fd events have been received we then queue up the notifiers callback as events

    server->on_read = [&server](EventLoop& loop) {
        int fd = server->accept();
        printf("accepted new cat %d\n", fd);

        // TODO: remove requirement for epollin/epollet
        loop.add_read(fd, [fd](EventLoop& loop) {
            Bytes buf(1024);

            auto rc = recv(fd, (void*)buf.data(), buf.size(), 0);
            if (rc <= 0) {
                printf("Connection closed!\n");
                loop.remove_read(fd);
                return;
            }

            std::string_view message((char const*)buf.data(), rc);
            std::cout << fd << ": " << message;
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

    loop.add_read(fileno(stdin), [](EventLoop const&) {
        printf("Received key press, shutting down.\n");
        exit(EXIT_SUCCESS);
    });

    loop.exec();

    return EXIT_SUCCESS;
}