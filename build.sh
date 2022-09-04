# optionally uses libmagic-dev for mimetype

g++ -g -Wall -std=c++20 tcpserver.cpp utility/EventLoop.cpp utility/TcpServer.cpp utility/Socket.cpp -D USE_LIBMAGIC -lmagic -o tcpserver
# g++ -g -Wall -std=c++20 tcpserver.cpp utility/EventLoop.cpp utility/TcpServer.cpp utility/Socket.cpp
