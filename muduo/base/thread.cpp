//
// Created by liucxi on 2022/10/17.
//

#include "thread.h"

#include <memory>

#include "current_thread.h"
#include "semaphore.h"

/// 不能使用 m_threadNum = 0
std::atomic_int Thread::m_threadNum(0);

Thread::Thread(std::function<void()> func)
        : m_started(false), m_joined(false), m_tid(0), m_func(std::move(func)) {
    ++m_threadNum;
}

Thread::~Thread() {
    /**
     * @details m_started 线程启动了，即真的创建了一个新的线程，
     *          !m_joined 没有被 join，因为 join 和 detach 只能选一个
     */
    if (m_started && !m_joined) {
        m_thread->detach();
    }
}

void Thread::start() {
    m_started = true;
    sem_t sem;
    sem_init(&sem, false, 0);
    // 开启线程
    m_thread = std::make_shared<std::thread>([&]() {
        m_tid = CurrentThread::tid();   //  获取线程的 tid 值
        sem_post(&sem);
        m_func();               // 新线程执行线程函数
    });
    /// 需要等待 tid 真正被赋值后才返回，这样用户在调用 Thread::start() 之后可以放心的访问 tid
    sem_wait(&sem);
}

void Thread::join() {
    m_joined = true;
    m_thread->join();
}