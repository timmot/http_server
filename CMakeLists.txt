cmake_minimum_required(VERSION 3.8)
project(http_server CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(APPLE)
    link_directories(/usr/local/Cellar/libmagic/5.45/lib/)
endif()

add_definitions(-DUSE_LIBMAGIC)
set(CMAKE_CXX_FLAGS "-Wall --pedantic -Wshadow -Wextra")

FILE(GLOB SOURCE_FILES utility/*.cpp http/*.cpp)

add_executable(server http_server.cpp ${SOURCE_FILES})
target_link_libraries(server magic)

add_executable(client http_client.cpp ${SOURCE_FILES})
add_executable(json json.cpp ${SOURCE_FILES})
add_executable(test test.cpp ${SOURCE_FILES})

#add_subdirectory(chip8)
