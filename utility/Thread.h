#include <functional>
#include <memory>
#include <pthread.h>

enum ThreadState {
    Created,
    Started,
    Joined,
    Detached,
    Exited,
};

class Thread {
public:
    static std::unique_ptr<Thread> create(std::function<void()> func)
    {
        return std::make_unique<Thread>(std::move(func));
    }

    void start();
    bool join();

    void detach();

    Thread(std::function<void()> func)
        : m_func(std::move(func))
    {
    }

    ~Thread();

private:
    pthread_t m_thread_id { 0 };
    std::function<void()> m_func;
    ThreadState m_state { Created };
};
