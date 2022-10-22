//
// Created by liucxi on 2022/10/17.
//

#ifndef MUDUO_THREAD_H
#define MUDUO_THREAD_H

#include "noncopyable.h"

#include <functional>
#include <thread>
#include <memory>
#include <unistd.h>
#include <atomic>

class Thread : NonCopyable {
public:
    using ThreadFunc = std::function<void()>;

    explicit Thread(ThreadFunc func);

    ~Thread();

    void start();

    void join();

    bool isStarted() const { return m_started; }

    pid_t getTid() const { return m_tid; }

    static int GetThreadNum() { return m_threadNum; }

private:
    bool m_started;
    bool m_joined;
    std::shared_ptr<std::thread> m_thread;          /// thread 一定义就在运行，所以使用智能指针控制启动时机
    pid_t m_tid;
    ThreadFunc m_func;
    static std::atomic_int m_threadNum;
};

#endif //MUDUO_THREAD_H
