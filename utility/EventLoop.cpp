// Copyright (c) 2022 - Tim Blackstone

#include "EventLoop.h"
#include <stdio.h>
#include <unistd.h>
#include <utility>

#ifdef __APPLE__
#include <sys/event.h>
#else
#include <sys/epoll.h>
#endif

static EventLoop* s_current_loop;

EventLoop::EventLoop()
{
#ifdef __APPLE__
    m_fd = kqueue();
#else
    m_fd = epoll_create1(0);
#endif
    s_current_loop = this;
}

EventLoop::~EventLoop()
{
    close(m_fd);
}

EventLoop& EventLoop::current()
{
    return *s_current_loop;
}

void EventLoop::exec()
{
    for (;;) {
        pump();
    }
}

void EventLoop::pump()
{
#ifdef __APPLE__
    struct kevent events[10];
    auto number_of_events = kevent(m_fd, nullptr, 0, events, 10, NULL);
#else
    epoll_event events[10];
    auto number_of_events = epoll_wait(m_fd, events, 10, 1000);
#endif

    if (number_of_events < 0) {
        // TODO: fail somehow?
    } else {
        for (int i = 0; i < number_of_events; ++i) {
#ifdef __APPLE__
            auto fd = events[i].ident;
#else
            auto fd = events[i].data.fd;
#endif
            if (m_fd_to_callback.find(fd) != m_fd_to_callback.end()) {
                m_fd_to_callback[fd](*this);
            } else {
                // nothing to do
            }
        }
    }
}

void EventLoop::add_read(int read_fd, std::function<void(EventLoop&)> callback)
{
#ifdef __APPLE__
    struct kevent new_event = {
        .ident = (uintptr_t)read_fd,
        .filter = EVFILT_READ,
        .flags = EV_ADD | EV_ENABLE
    };

    kevent(m_fd, &new_event, 1, NULL, 0, NULL);
#else
    epoll_event ev = {};
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = read_fd;
    if (epoll_ctl(m_fd, EPOLL_CTL_ADD, read_fd, &ev) == -1) {
        perror("epoll_ctl");
    }
#endif

    m_fd_to_callback.emplace(read_fd, callback);
}

void EventLoop::remove_read(int read_fd)
{
    close(read_fd);

#ifdef __APPLE__
    // From kqueue manpage:
    // Events which are attached to file descriptors are automatically deleted on the last close of the descriptor.

    struct kevent old_event = {
        .ident = (uintptr_t)read_fd,
        .filter = EVFILT_READ,
        .flags = EV_DELETE
    };

    kevent(m_fd, &old_event, 1, 0, 0, 0);
#else
    epoll_ctl(m_fd, EPOLL_CTL_DEL, read_fd, NULL);
#endif

    auto entry = m_fd_to_callback.find(read_fd);

    if (entry != m_fd_to_callback.end())
        m_fd_to_callback.erase(entry);
}