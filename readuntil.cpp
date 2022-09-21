#include "utility/Bytes.hpp"
#include "utility/EventLoop.h"
#include "utility/Socket.h"
#include "utility/TcpServer.h"
#include "utility/log.hpp"
#include <iostream>
#include <memory>
#include <regex>
#include <vector>

int main()
{
    auto server = TcpServer::create();

    EventLoop main_loop;

    server->listen("0.0.0.0", 10000);
    if (server.has_value())
        logln("Listening on port 10000");
    else
        return 1;

    // TODO: Need to use a buffered socket
    // e.g. https://github.com/SerenityOS/serenity/blob/master/Userland/Libraries/LibCore/Stream.h#L798
    // e.g. https://github.com/python/cpython/blob/main/Lib/asyncio/streams.py#L556

    // Do we have to set up on_read to do this buffering of reads?

    // Serenity static_casts their Socket implementation to their BufferedSocket implementation
    // Which I suppose is acceptable because BufferedSocket inherits Socket?

    // Implementation
    // 1. each time a BufferedSocket has read() called, see if the buffer needs to be refilled

    // check if eof?
    // check if readable?
    //

    std::vector<BufferedSocket> clients;
    server->on_read = [&server, &clients](EventLoop& loop) {
        clients.emplace_back(BufferedSocket { server->accept() });
        auto& client = clients.back();
        logln("Accepted new client {}", client.fd());

        loop.add_read(client.fd(), [&client, &clients](EventLoop& sub_loop) {
            while (true) {
                auto maybe_bytes = client.read_line();

                if (!maybe_bytes.has_value()) {
                    int fd = client.fd();
                    sub_loop.remove_read(client.fd());
                    clients.erase(std::remove_if(clients.begin(), clients.end(), [&client](auto& client_val) { return client_val.fd() == client.fd(); }));

                    logln("Connection closed for {}, {} clients remaining", fd, clients.size());
                    break;
                } else if (maybe_bytes.has_value() && maybe_bytes->size() > 0) {
                    auto pretty_text = std::string(reinterpret_cast<char*>(maybe_bytes->data()), maybe_bytes->size());
                    pretty_text[maybe_bytes->size() - 1] = 'n';
                    pretty_text[maybe_bytes->size() - 2] = 'r';

                    // FIXME: Prints \n instead of size when requesting with curl
                    // logln("{}: {}", maybe_bytes->size(), pretty_text);

                    printf("%ld: %s\n", maybe_bytes->size(), pretty_text.c_str());

                    if (maybe_bytes->data()[0] == '\r' && maybe_bytes->data()[1] == '\n')
                        client.write({ "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 17\r\nConnection: close\r\n\r\npee pee poo poo\r\n" });
                }

                // client->write(std::to_string(nread) + "\n");
            }
        });
    };

    main_loop.exec();

    return 0;
}
