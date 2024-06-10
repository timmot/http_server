#include "Thread.h"

void Thread::start()
{
    pthread_create(
        &m_thread_id, nullptr, [](void* arg) -> void* {
            auto self = static_cast<Thread*>(arg);
            self->m_func();
            if (self->m_state != ThreadState::Detached)
                self->m_state = ThreadState::Exited;

            return nullptr;
        },
        this);

    m_state = ThreadState::Started;
}

bool Thread::join()
{
    if (m_state == ThreadState::Detached) {
        printf("Thread error: Can't join a thread after detaching.\n");
        return false;
    }

    auto rc = pthread_join(m_thread_id, NULL);

    if (m_state != ThreadState::Exited) {
        printf("Thread error: Thread was not exited after being joined, callback failed?\n");
        return false;
    }

    m_state = ThreadState::Joined;

    return rc == 0;
}

void Thread::detach()
{
    auto rc = pthread_detach(m_thread_id);
    if (rc == 0)
        m_state = ThreadState::Detached;
}

Thread::~Thread()
{
    if (m_state == ThreadState::Started || m_state == ThreadState::Exited) {
        printf("Thread error: Forcing a join, thread is being destructed without being detached or join.\n");
        this->join();
    }
}
