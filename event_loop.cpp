//
// Created by liucxi on 2022/10/12.
//

#include "event_loop.h"

#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>

#include "muduo/base/logger.h"
#include "muduo/base/current_thread.h"
#include "muduo/net/poller/poller.h"
#include "muduo/net/channel.h"

thread_local EventLoop *t_loopInThisThread = nullptr;

// 定义默认的 Poller 超时事件
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
        : m_looping(false), m_quit(false), m_callingPendingFunctors(false), m_threadId(currentPid()),
          m_poller(Poller::NewDefaultPoller(this)), m_wakeupFd(createEventFd()),
          m_wakeupChannel(new Channel(this, m_wakeupFd)) {
    if (t_loopInThisThread) {
        LOG_FATAL("Another EventLoop exits in this thread %d \n", m_threadId);
    } else {
        t_loopInThisThread = this;
    }

    // 设置 wakeup fd
    m_wakeupChannel->setReadCallback(std::bind(&EventLoop::handleRead, this));
    m_wakeupChannel->enableReading();
}

EventLoop::~EventLoop() {
    m_wakeupChannel->disableAll();
    m_wakeupChannel->remove();
    ::close(m_wakeupFd);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
    m_looping = true;
    m_quit = false;

    while (!m_quit) {
        m_activeChannels.clear();
        m_pollRetTime = m_poller->pool(kPollTimeMS, m_activeChannels);
        for (auto channel : m_activeChannels) {
            channel->handleEvent(m_pollRetTime);
        }
        doPendingFunctors();
    }
    m_looping = false;
}

void EventLoop::quit() {
    m_quit = true;
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

void EventLoop::runInLoop(Functor cb) {
    if (isInLoopThread()) {
        cb();
    } else {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(Functor cb) {
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_pendingFunctors.push_back(std::move(cb));
    }

    if (!isInLoopThread() || m_callingPendingFunctors) {
        wakeup();
    }
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = ::write(m_wakeupFd, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR("%s error \n", __FUNCTION__);
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
    std::vector<Functor> functors;
    m_callingPendingFunctors = true;

    {
        std::unique_lock<std::mutex> lock(m_mutex);
        functors.swap(m_pendingFunctors);
    }

    for (const auto& functor : functors) {
        functor();
    }

    m_callingPendingFunctors = false;
}
