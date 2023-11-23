cmake_minimum_required(VERSION 3.8)

set(CMAKE_MODULE_PATH ../cmake)

find_package(SDL2 REQUIRED)
find_package(SDL2TTF REQUIRED)

include_directories(chip8 ${SDL2_INCLUDE_DIRS} ${SDL2TTF_INCLUDE_DIR})
add_executable(chip8 chip8.cpp ${SOURCE_FILES})

target_link_libraries(chip8 ${SDL2_LIBRARIES} ${SDL2TTF_LIBRARY}) 