//
// Created by liucxi on 2022/10/12.
//

#include "event_loop.h"

#include <sys/eventfd.h>
#include <unistd.h>

#include "../base/logger.h"
#include "poller/poller.h"
#include "channel.h"

/// 防止一个线程创建多个 EventLoop
thread_local EventLoop *t_loopInThisThread = nullptr;

// 定义默认的 Poller 超时时间
const int kPollTimeMS = 10000;

// 创建 wakeup fd，用来唤醒 subReactor 处理新来的连接
int createEventFd() {
    int evtFd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtFd < 0) {
        LOG_FATAL("%s error \n", __FUNCTION__);
    }
    return evtFd;
}

EventLoop::EventLoop()
        : m_quit(false), m_callingPendingFunctors(false), m_threadId(currentTid()),
          m_wakeupFd(createEventFd()), m_poller(Poller::NewDefaultPoller(this)),
          m_wakeupChannel(new Channel(this, m_wakeupFd)) {
    if (t_loopInThisThread) {
        LOG_FATAL("Another EventLoop exits in this thread %d \n", m_threadId);
    } else {
        t_loopInThisThread = this;
    }

    // 设置 wakeup fd 的事件类型和发生事件后的回调函数
    m_wakeupChannel->enableReading();
    m_wakeupChannel->setReadCallback(std::bind(&EventLoop::handleRead, this));
}

EventLoop::~EventLoop() {
    m_wakeupChannel->disableAll();
    m_wakeupChannel->remove();
    ::close(m_wakeupFd);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
    m_quit = false;
    LOG_INFO("EventLoop %p start looping \n", this);
    while (!m_quit) {
        m_activeChannels.clear();
        m_pollRetTime = m_poller->pool(kPollTimeMS, m_activeChannels);
        for (auto channel: m_activeChannels) {
            /**
             * @details m_poller->pool() 用来监听哪些 channel 发生事件了
             * 然后发生事件的 channel 依次处理所绑定 fd 上发生的事件（执行对应的回调函数）
             * 包括多个与客户端直接通信的 fd 和 wakeup fd.
             */
            channel->handleEvent();
        }
        /**
         * 当前 EventLoop 事件循环需要处理的回调操作
         * mainLoop 主要执行 accept 逻辑，创建新的客户端连接，然后分发给 subLoop
         * mainLoop 事先在 subLoop 注册一个回调，subLoop 被唤醒之后，执行被注册的回调
         */
        doPendingFunctors();
    }
    LOG_INFO("EventLoop %p stop looping \n", this);
}

void EventLoop::quit() {
    m_quit = true;
    /// 如果在其他线程中调用本 EventLoop 的 quit
    if (!isInLoopThread()) {
        wakeup();
    }
}

void EventLoop::handleRead() {
    uint64_t one = 1;
    ssize_t n = ::read(m_wakeupFd, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR("%s error \n", __FUNCTION__);
    }
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = ::write(m_wakeupFd, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR("%s error \n", __FUNCTION__);
    }
}

void EventLoop::runInLoop(Functor cb) {
    if (isInLoopThread()) {
        cb();                               // 在当前的 loop 线程中直接执行
    } else {
        queueInLoop(std::move(cb));   // 在非当前线程中执行 cb，就需要唤醒 loop 所在线程，再执行 cb
    }
}

void EventLoop::queueInLoop(Functor cb) {
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_pendingFunctors.push_back(std::move(cb));
    }
    /**
     * 唤醒执行该 cb 的 loop 所在的线程
     * 1. 执行 cb 的 loop 所在线程不是当前线程
     * 2. 执行 cb 的 loop 正在执行上一轮需要执行的 cb, 这时又添加了新的 cb, 若是去掉 m_callingPendingFunctors，
     *    则新添加的 cb 需要等待 m_poller->pool 再次返回
     */
    if (!isInLoopThread() || m_callingPendingFunctors) {
        wakeup();
    }
}

void EventLoop::updateChannel(Channel *channel) {
    m_poller->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel) {
    m_poller->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel) {
    return m_poller->hasChannel(channel);
}

void EventLoop::doPendingFunctors() {
    // 保存需要执行的回调函数，避免加锁时间过长
    std::vector<Functor> functors;
    m_callingPendingFunctors = true;
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        functors.swap(m_pendingFunctors);
    }
    for (const auto &functor: functors) {
        functor();
    }
    m_callingPendingFunctors = false;
}
