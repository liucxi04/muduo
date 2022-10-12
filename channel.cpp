//
// Created by liucxi on 2022/10/12.
//

#include "channel.h"

#include "event_loop.h"
#include "logger.h"

#include <sys/epoll.h>

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop *loop, int fd)
        : m_loop(loop), m_fd(fd), m_events(0), m_retEvents(0), m_index(-1), m_tied(false) {
}


void Channel::tie(const std::shared_ptr<void> &obj) {
    m_tie = obj;
    m_tied = true;
}

void Channel::update() {
    m_loop->updateChannel(this);
}

void Channel::remove() {
    m_loop->removeChannel(this);
}

void Channel::handleEvent(const Time &recvTime) {
    if (m_tied) {
        std::shared_ptr<void> guard;
        guard = m_tie.lock();
        if (guard) {
            handleEventWithGuard(recvTime);
        }
    } else {
        handleEventWithGuard(recvTime);
    }
}

void Channel::handleEventWithGuard(const Time &recvTime) {
    LOG_INFO("channel handelEvent ret events: %d\n", m_retEvents);

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
            m_readCallback(recvTime);
        }
    }

    if (m_retEvents & EPOLLOUT) {
        if (m_writeCallback) {
            m_writeCallback();
        }
    }
}
