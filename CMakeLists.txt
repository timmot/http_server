cmake_minimum_required(VERSION 3.8)
project(http_server CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_definitions(-DUSE_LIBMAGIC)
set(CMAKE_CXX_FLAGS "-Wall --pedantic -Wshadow -Wextra")

FILE(GLOB SOURCE_FILES utility/*.cpp http/*.cpp)

add_executable(server tcpserver.cpp ${SOURCE_FILES})
target_link_libraries(server magic)

add_executable(client tcpclient.cpp ${SOURCE_FILES})
# add_executable(json tcpserver.cpp ${SOURCE_FILES})
