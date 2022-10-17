//
// Created by liucxi on 2022/10/12.
//

#ifndef MUDUO_EVENT_LOOP_H
#define MUDUO_EVENT_LOOP_H

#include "muduo/base/noncopyable.h"
#include "muduo/base/times.h"
#include "muduo/base/current_thread.h"

#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>

class Poller;

class Channel;

class EventLoop : NonCopyable {
public:
    using Functor = std::function<void()>;

    EventLoop();

    ~EventLoop();

    void loop();

    void quit();

    Time getPollRetTime() const { return m_pollRetTime; }

    void runInLoop(Functor cb);

    void queueInLoop(Functor cb);

    void wakeup();

    void updateChannel(Channel *channel);

    void removeChannel(Channel *channel);

    bool hasChannel(Channel *channel);

    bool isInLoopThread() const { return m_threadId == currentPid(); }

private:
    void handleRead();

    void doPendingFunctors();

private:
    std::atomic_bool m_looping;
    std::atomic_bool m_quit;

    const pid_t m_threadId;
    Time m_pollRetTime;

    std::unique_ptr<Poller> m_poller;

    int m_wakeupFd;                             // 轮询唤醒 poller 的标志
    std::unique_ptr<Channel> m_wakeupChannel;
    std::vector<Channel *> m_activeChannels;

    std::atomic_bool m_callingPendingFunctors;  // 标识当前 loop 是否有需要执行的回调操作
    std::vector<Functor> m_pendingFunctors;     // 存储 loop 需要执行的回调操作

    std::mutex m_mutex;
};

#endif //MUDUO_EVENT_LOOP_H
