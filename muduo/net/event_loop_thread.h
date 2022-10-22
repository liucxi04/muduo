//
// Created by liucxi on 2022/10/22.
//

#ifndef MUDUO_EVENT_LOOP_THREAD_H
#define MUDUO_EVENT_LOOP_THREAD_H

#include "../base/noncopyable.h"
#include "../base/thread.h"

#include <functional>
#include <mutex>
#include <condition_variable>

class EventLoop;

class EventLoopThread : NonCopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    explicit EventLoopThread(ThreadInitCallback callback);

    ~EventLoopThread();

    EventLoop *startLoop();

private:
    /**
     * @brief m_thread 内部执行的函数
     */
    void ThreadFunc();

private:
    bool m_exiting;
    EventLoop *m_loop;
    Thread m_thread;
    std::mutex m_mutex;
    std::condition_variable m_cond;
    ThreadInitCallback m_callback;
};


#endif //MUDUO_EVENT_LOOP_THREAD_H
