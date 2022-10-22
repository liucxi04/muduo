//
// Created by liucxi on 2022/10/12.
//

#include "channel.h"

#include "event_loop.h"
#include "../base/logger.h"

#include <sys/epoll.h>

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop *loop, int fd)
        : m_loop(loop), m_fd(fd), m_events(0), m_retEvents(0), m_index(-1), m_tied(false) {
}

/**
 * TODO 该方法在哪里调用
 */
void Channel::tie(const std::shared_ptr<void> &obj) {
    m_tie = obj;
    m_tied = true;
}

/**
 * @brief 通知 poller 更新感兴趣的事件，需要通过两者同属的 EventLoop 实现。
 */
void Channel::update() {
    m_loop->updateChannel(this);
}

/**
 * @brief 从 poller 中移除该 channel，需要通过两者同属的 EventLoop 实现。
 */
void Channel::remove() {
    m_loop->removeChannel(this);
}

/**
 * @brief 如果绑定过观察对象，只有在所监听的对象还存在时，才会处理
 */
void Channel::handleEvent() {
    if (m_tied) {
        std::shared_ptr<void> guard;
        guard = m_tie.lock();
        if (guard) {
            handleEventWithGuard();
        }
    } else {
        handleEventWithGuard();
    }
}

void Channel::handleEventWithGuard() {
    LOG_INFO("%s run. retEvents is %d \n",__FUNCTION__, m_retEvents);

    if ((m_retEvents & EPOLLHUP) && !(m_retEvents & EPOLLIN)) {
        if (m_closeCallback) {
            m_closeCallback();
        }
    }

    if (m_retEvents & EPOLLERR) {
        if (m_errorCallback) {
            m_errorCallback();
        }
    }

    if (m_retEvents & (EPOLLIN | EPOLLPRI)) {
        if (m_retEvents) {
            m_readCallback();
        }
    }

    if (m_retEvents & EPOLLOUT) {
        if (m_writeCallback) {
            m_writeCallback();
        }
    }
}
