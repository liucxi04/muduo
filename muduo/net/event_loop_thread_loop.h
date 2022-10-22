//
// Created by liucxi on 2022/10/22.
//

#ifndef MUDUO_EVENT_LOOP_THREAD_LOOP_H
#define MUDUO_EVENT_LOOP_THREAD_LOOP_H

#include "../base/noncopyable.h"

#include <functional>
#include <vector>
#include <memory>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : NonCopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    explicit EventLoopThreadPool(EventLoop* baseLoop);

    ~EventLoopThreadPool() = default;

    void start(const ThreadInitCallback &cb = ThreadInitCallback());

    /**
     * @brief 以轮询方式分配 channel
     */
    EventLoop *getNextLoop();

    bool isStarted() const { return m_started; }

    void setThreadNum(int threadNum) { m_threadNum = threadNum; }
private:
    bool m_started;
    EventLoop* m_baseLoop;
    int m_threadNum;
    int m_next;
    std::vector<EventLoop*> m_loops;
    std::vector<std::unique_ptr<EventLoopThread>> m_threads;
};

#endif //MUDUO_EVENT_LOOP_THREAD_LOOP_H
