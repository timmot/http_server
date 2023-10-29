#g++-10 -g -O0 -Wall --pedantic -Wshadow -Wextra -std=c++20 0-smoketest.cpp utility/EventLoop.cpp utility/TcpServer.cpp utility/Socket.cpp -o 0-smoketest
#g++-10 -g -O0 -Wall --pedantic -Wshadow -Wextra -std=c++20 1-primetime.cpp utility/EventLoop.cpp utility/TcpServer.cpp utility/Socket.cpp -o 1-primetime
#g++-10 -g -O0 -Wall --pedantic -Wshadow -Wextra -std=c++20 2-meanstoanend.cpp utility/EventLoop.cpp utility/TcpServer.cpp utility/Socket.cpp -o 2-meanstoanend
g++-10 -g -O0 -Wall --pedantic -Wshadow -Wextra -std=c++20 readuntil.cpp utility/EventLoop.cpp utility/TcpServer.cpp utility/Socket.cpp -o readuntil
#g++-10 -g -O0 -Wall --pedantic -Wshadow -Wextra -std=c++20 tcpserver.cpp utility/EventLoop.cpp utility/TcpServer.cpp utility/Socket.cpp -o httpserver
#g++-10 -g -O0 -Wall --pedantic -Wshadow -Wextra -std=c++20 tcpclient.cpp utility/EventLoop.cpp utility/TcpServer.cpp utility/Socket.cpp -o httpclient
#g++-10 -g -O0 -Wall --pedantic -Wshadow -Wextra -std=c++20 -DUSE_LIBMAGIC main.cpp -lmagic -o test