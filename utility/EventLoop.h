// Copyright (c) 2022 - Tim Blackstone

#pragma once

#include <functional>
#include <stdint.h>
#include <unordered_map>

class EventLoop {
public:
    EventLoop();
    ~EventLoop();

    static EventLoop& current();

    void exec();
    void pump();

    void add_read(int read_fd, std::function<void(EventLoop&)> callback);
    void remove_read(int read_fd);

private:
    int m_fd;
    std::unordered_map<int, std::function<void(EventLoop&)>> m_fd_to_callback;
};