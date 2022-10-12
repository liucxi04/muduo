//
// Created by liucxi on 2022/10/12.
//

#include "epoll_poller.h"
#include "logger.h"
#include "times.h"
#include "channel.h"

#include <vector>
#include <cerrno>
#include <unistd.h>

const int kNew = -1;
const int kAdd = 1;
const int kDel = 2;

EpollPoller::EpollPoller(EventLoop *loop)
    : Poller(loop)
    , m_epfd(::epoll_create1(EPOLL_CLOEXEC))
    , m_events(kInitEventListSize) {
    if (m_epfd < 0) {
        LOG_FATAL("epoll_create error: %d\n", errno);
    }
}

EpollPoller::~EpollPoller() {
    ::close(m_epfd);
}

Time EpollPoller::pool(Time timeoutMs, std::vector<Channel *> &activeChannels) {
    return Time();
}

void EpollPoller::updateChannel(Channel *channel) {

}

void EpollPoller::removeChannel(Channel *channel) {

}

void EpollPoller::fillActiveChannels(int numEvents, std::vector<Channel> &activeChannels) const {

}

void EpollPoller::update(int op, Channel *channel) {

}
