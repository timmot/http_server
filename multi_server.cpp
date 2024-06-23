#include "utility/Ipv4Address.hpp"
#include "utility/Thread.h"
#include "utility/log.hpp"
#include <arpa/inet.h>
#include <fcntl.h>
#include <string>
#include <sys/epoll.h>
#include <vector>

int main()
{
    auto loop_fd = epoll_create1(0);
    int sock;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        logln("cant make socket 1");
        return 1;
    }
    // Set reusable
    int yes = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        logln("cant make socket 2");
        return 1;
    }

    // Set non blocking
    // int sock_opts = fcntl(sock, F_GETFL);
    // fcntl(sock, F_SETFL, sock_opts | O_NONBLOCK);

    auto host = Ipv4Address::resolve_hostname("localhost");
    sockaddr_in address = {};
    address.sin_family = AF_INET;
    address.sin_port = htons(8000);
    address.sin_addr.s_addr = host->to_in_addr_t();

    if (bind(sock, (const sockaddr*)&address, sizeof address) == -1) {
        perror("server: bind");
        return 1;
    }

    if (::listen(sock, 10) == -1) {
        printf("couldn't listen");
        return 1;
    }

stop:
    logln("new client pls");
    sockaddr_storage their_addr;
    socklen_t addr_size = sizeof their_addr;
    int new_fd = accept(sock, (sockaddr*)&their_addr, &addr_size);

    epoll_event ev = {};
    ev.events = EPOLLIN | EPOLLET | EPOLLHUP | EPOLLRDHUP;
    /*
                  epoll_wait(2) will always wait for this event; it is not
                  necessary to set it in events when calling epoll_ctl().*/
    ev.data.fd = new_fd;
    if (epoll_ctl(loop_fd, EPOLL_CTL_ADD, new_fd, &ev) == -1) {
        perror("epoll_ctl");
    }

    for (;;) {
        epoll_event events[10];
        auto number_of_events = epoll_wait(loop_fd, events, 10, 1000);

        if (number_of_events < 0) {
            perror("epoll_wait");
            return 1;
        } else {
            for (int i = 0; i < number_of_events; ++i) {
                if (events[i].events & EPOLLIN) {
                    logln("event readable");
                    char buf[1024];
                    int rc = recv(new_fd, buf, 1024, 0);
                    logln("recv({}): {}", rc, std::string { buf, 1024 });
                }

                if (events[i].events & EPOLLHUP) {
                    logln("{} event hungup :(", new_fd);
                    close(new_fd);
                    goto stop;
                } else if (events[i].events & EPOLLRDHUP) {
                    logln("{} event writer hungup :(", new_fd);
                    char buf[1024];
                    int rc = recv(new_fd, buf, 1024, 0);
                    logln("recv({}): {}", rc, std::string { buf, 1024 });

                    close(new_fd);
                    goto stop;
                }
            }
        }
    }

    // char buf[1024];
    // int rc = recv(new_fd, buf, 1024, 0);
    // logln("recv ({}): {}", rc, std::string { buf, 1024 });
}
