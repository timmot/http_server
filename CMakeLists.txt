cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(USE_LIBMAGIC True)

# to set for just one target
# set_property(TARGET out PROPERTY CXX_STANDARD 20)
project(http_server CXX)

FILE(GLOB SOURCE_FILES utility/*.cpp http/*.cpp)

add_executable(server tcpserver.cpp ${SOURCE_FILES})
add_executable(client tcpserver.cpp ${SOURCE_FILES})
# add_executable(json tcpserver.cpp ${SOURCE_FILES})

# target_link_libraries(out ws2_32)
target_link_libraries(server magic)
add_compile_options(-Wall --pedantic -Wshadow -Wextra)
