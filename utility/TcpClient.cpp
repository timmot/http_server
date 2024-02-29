#include "TcpClient.h"
#include "EventLoop.h"
#include "Ipv4Address.hpp"
#include <fcntl.h>
#include <sys/socket.h>

std::optional<TcpClient> TcpClient::create()
{
    int fd = socket(PF_INET, SOCK_STREAM, 0);
    if (fd == -1)
        return {};

    return { fd };
}

TcpClient::TcpClient(int socket_fd)
    : BufferedSocket(socket_fd)
{
    EventLoop::current().add_read(fd(), [this](EventLoop& loop) {
        if (on_read) {
            Bytes message(4096);
            auto maybe_response = read(message);

            if (maybe_response) {
                on_read(loop, std::move(*maybe_response));
            } else {
                // TODO: received 0 bytes, close socket?
                loop.remove_read(fd());
            }
        }
    });
}

bool TcpClient::connect(Ipv4Address host, int port)
{
    sockaddr_in address = {};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = host.to_in_addr_t();

    if (::connect(fd(), (const sockaddr*)&address, sizeof address)) {
        perror("connect");
        return false;
    }

    // Set non-blocking after connect so it is synchronous
    int sock_opts = fcntl(fd(), F_GETFL);
    fcntl(fd(), F_SETFL, sock_opts | O_NONBLOCK);

    return true;
}