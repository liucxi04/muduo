//
// Created by liucxi on 2022/10/22.
//
#include "event_loop_thread_pool.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop)
    : m_started(false), m_baseLoop(baseLoop), m_threadNum(0), m_next(0) {
}

void EventLoopThreadPool::start(const ThreadInitCallback &cb) {
    m_started = true;

    for (int i = 0; i < m_threadNum; ++i) {
        auto *t = new EventLoopThread(cb);
        m_threads.push_back(std::unique_ptr<EventLoopThread>(t));
        m_loops.push_back(t->startLoop());
    }

    // 只有一个 mainLoop 的情况
    if (m_threadNum == 0 && cb) {
        cb(m_baseLoop);
    }
}

EventLoop *EventLoopThreadPool::getNextLoop() {
    EventLoop *loop = m_baseLoop;
    if (!m_loops.empty()) {
        loop = m_loops[m_next];
        ++m_next;
        if (m_next >= m_loops.size()) {
            m_next = 0;
        }
    }
    return loop;
}



