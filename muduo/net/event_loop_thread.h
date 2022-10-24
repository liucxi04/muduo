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

/**
 * one loop per thread
 */
class EventLoopThread : NonCopyable {
public:
    // 线程初始化回调函数，在线程创建完成 EventLoop 之后调用
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    /**
     * @brief 构造函数
     * @param callback 线程初始化回调
     */
    explicit EventLoopThread(ThreadInitCallback callback);

    ~EventLoopThread();

    /**
     * @details 启动线程，等待 EventLoop 创建完成并返回 loop
     * @return
     */
    EventLoop *startLoop();

private:
    /**
     * @brief m_thread 内部执行的函数
     */
    void ThreadFunc();

private:
    EventLoop *m_loop;              // one loop
    Thread m_thread;                // per thread
    ThreadInitCallback m_callback;
    std::mutex m_mutex;
    std::condition_variable m_cond;
};


#endif //MUDUO_EVENT_LOOP_THREAD_H
