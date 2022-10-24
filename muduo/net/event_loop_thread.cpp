//
// Created by liucxi on 2022/10/22.
//

#include "event_loop_thread.h"
#include "event_loop.h"

EventLoopThread::EventLoopThread(ThreadInitCallback callback)
        : m_loop(nullptr)
        , m_thread(std::bind(&EventLoopThread::ThreadFunc, this))
        , m_callback(std::move(callback)) {
}

EventLoopThread::~EventLoopThread() {
    if (m_loop != nullptr) {
        m_loop->quit();
        m_thread.join();
    }
}

EventLoop *EventLoopThread::startLoop() {
    // 创建一个新线程，线程函数为 EventLoopThread::ThreadFunc()
    m_thread.start();

    // 等待新线程创建完成 loop
    EventLoop *loop;
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_loop == nullptr) {
            m_cond.wait(lock);
        }
        loop = m_loop;
    }
    return loop;
}

void EventLoopThread::ThreadFunc() {
    // 一个新的线程运行本函数，在开始时创建一个 EventLoop，并通知创建该线程的父线程
    EventLoop loop;         /// 创建一个新的 loop，和上面的线程一一对应，该 loop 创建在栈上，省去了后续手动释放的麻烦
    // EventLoop 创建完成执行一些指定的初始化操作，可以没有
    if (m_callback) {
        m_callback(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_loop = &loop;                                     /// 有被转义的危险？ 因为下面的 loop 事件循环不会退出, 所以没有问题
        m_cond.notify_one();
    }
    m_loop->loop();         /// 开启事件循环
    // 一般执行不到下面
    std::unique_lock<std::mutex> lock(m_mutex);
    m_loop = nullptr;
}