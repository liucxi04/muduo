//
// Created by liucxi on 2022/10/12.
//

#ifndef MUDUO_EVENT_LOOP_H
#define MUDUO_EVENT_LOOP_H

#include "../base/noncopyable.h"
#include "../base/times.h"
#include "../base/current_thread.h"

#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>

class Poller;

class Channel;

/**
 * @brief Reactor, one loop per thread.
 */
class EventLoop : NonCopyable {
public:
    using Functor = std::function<void()>;          // 回调函数类型

    EventLoop();

    ~EventLoop();

    /**
     * @brief 开启事件循环
     */
    void loop();

    /**
     * @brief 退出事件循环
     */
    void quit();

    Time getPollRetTime() const { return m_pollRetTime; }

    /**
     * @brief 在当前 loop 执行 cb
     */
    void runInLoop(Functor cb);

    /**
     * @brief 把 cb 放入队列, 唤醒 loop 所在的线程，执行 cb
     */
    void queueInLoop(Functor cb);

    /**
     * @brief mainReactor 唤醒 subReactor (唤醒 loop 所在线程)
     */
    void wakeup();

    void updateChannel(Channel *channel);

    void removeChannel(Channel *channel);

    bool hasChannel(Channel *channel);

    bool isInLoopThread() const { return m_threadId == currentTid(); }

private:
    /**
     * @brief 接受唤醒
     */
    void handleRead();

    /**
     * @brief 执行回调函数
     */
    void doPendingFunctors();

private:
    std::atomic_bool m_quit;                    // 标识当前 EventLoop 是否退出
    std::atomic_bool m_callingPendingFunctors;  // 标识当前 EventLoop 是否有需要执行的回调操作
    const pid_t m_threadId;                     // EventLoop 所属线程的 id, EventLoop 监听的 fd 必须在创建 EventLoop 的线程上执行
    Time m_pollRetTime;                         // poller 返回的发生事件的时间

    int m_wakeupFd;  /// 统一事件源, eventfd 用于线程间的 wait/notify, 当 mainLoop 获取一个新的客户连接的 channel，轮询唤醒 subLoop
    std::unique_ptr<Channel> m_wakeupChannel;   /// m_wakeupFd 的封装

    std::unique_ptr<Poller> m_poller;           /// EventLoop 所管理的 Poller
    std::vector<Channel *> m_activeChannels;    /// EventLoop 所管理的所有 Channel

    std::vector<Functor> m_pendingFunctors;     /// 存储 EventLoop 需要执行的回调操作
    std::mutex m_mutex;                         /// 对 m_pendingFunctors 操作的保护
};

#endif //MUDUO_EVENT_LOOP_H
