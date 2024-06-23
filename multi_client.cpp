#include "utility/EventLoop.h"
#include "utility/Ipv4Address.hpp"
#include "utility/TcpClient.h"
#include "utility/Thread.h"
#include "utility/log.hpp"
#include <arpa/inet.h>
#include <vector>

int main()
{
    std::vector<std::unique_ptr<Thread>> threads;

    for (int j = 0; j < 100; j++) {
        for (int i = 0; i < 100; i++) {
            auto t1 = Thread::create([]() {
                EventLoop loop;
                auto client = TcpClient::create();
                if (!client.has_value())
                    return;

                auto host = Ipv4Address::resolve_hostname("localhost");
                client->connect(*host, 8000);

                // client->set_timeout({ .tv_sec = 0, .tv_usec = 100 });

                Bytes data("hello\n");
                client->write(data);
            });
            threads.push_back(std::move(t1));
        }

        for (auto const& t : threads) {
            t->start();
        }
        for (auto const& t : threads) {
            t->join();
        }
    }

    logln("finished :)");
}
