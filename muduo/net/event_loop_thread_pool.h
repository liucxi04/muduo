//
// Created by liucxi on 2022/10/22.
//

#ifndef MUDUO_EVENT_LOOP_THREAD_POOL_H
#define MUDUO_EVENT_LOOP_THREAD_POOL_H

#include "../base/noncopyable.h"
#include "event_loop_thread.h"

#include <functional>
#include <vector>
#include <memory>

class EventLoop;

/**
 * @brief 事件线程池
 */
class EventLoopThreadPool : NonCopyable {
public:
    // 线程初始化回调函数，传递给 EventLoopThread
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    explicit EventLoopThreadPool(EventLoop* baseLoop);

    ~EventLoopThreadPool() = default;

    /**
     * @brief 启动所有线程, 开启事件循环
     * @param cb 线程初始化回调函数
     */
    void start(const ThreadInitCallback &cb = ThreadInitCallback());

    /**
     * @brief 以轮询方式分配 channel
     */
    EventLoop *getNextLoop();

    bool isStarted() const { return m_started; }

    void setThreadNum(int threadNum) { m_threadNum = threadNum; }
private:
    bool m_started;
    EventLoop* m_baseLoop;      // 用户创建的 loop, 也即 baseLoop, mainLoop. 如果不调用 setThreadNum, 就只有这一个 EventLoop
    int m_threadNum;            // 线程数量
    int m_next;                 // 轮询标识
    std::vector<EventLoop*> m_loops;        // subLoop, 因为是创建在栈上的, 所以不需要特别管理资源
    std::vector<std::unique_ptr<EventLoopThread>> m_threads;        // 线程队列，使用智能指针管理资源
};

#endif //MUDUO_EVENT_LOOP_THREAD_POOL_H
