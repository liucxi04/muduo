//
// Created by liucxi on 2022/10/12.
//

#include "epoll_poller.h"
#include "../../base/logger.h"
#include "../channel.h"

#include <vector>
#include <cerrno>
#include <unistd.h>
#include <cstring>

const int kNew = -1;    // 从来没有将该 channel 添加到 poller
const int kAdd = 1;     // 该 channel 已经添加到 poller
const int kDel = 2;     // 该 channel 已经从 poller 删除

EpollPoller::EpollPoller(EventLoop *loop)
    : Poller(loop)
    , m_epfd(::epoll_create1(EPOLL_CLOEXEC))
    , m_events(kInitEventListSize) {
    if (m_epfd < 0) {
        LOG_FATAL("%s error! \n", __FUNCTION__);
    }
}

EpollPoller::~EpollPoller() {
    ::close(m_epfd);
}

Time EpollPoller::pool(int timeoutMs, std::vector<Channel *> &activeChannels) {

    int numEvents = ::epoll_wait(m_epfd, &*m_events.begin(),
                         static_cast<int>(m_events.size()), timeoutMs);
    int saveError = errno;      /// 多个线程的 EpollPoller::pool 都可能访问 errno

    if (numEvents > 0) {
        LOG_INFO("%d events happened \n", numEvents);
        fillActiveChannels(numEvents, activeChannels);
        if (numEvents == m_events.size()) {
            m_events.resize(numEvents * 2);
        }
    } else if (numEvents == 0) {
        LOG_INFO("epoll_wait timeout \n");
    } else {
        if (saveError != EINTR) {
            errno = saveError;
            LOG_ERROR("epoll_wait error \n");
        }
    }
    return Time::now();
}

void EpollPoller::updateChannel(Channel *channel) {
    const int index = channel->getIndex();
    LOG_INFO("%s, fd = %d, events = %d, index = %d \n",
             __FUNCTION__, channel->getFd(), channel->getEvents(), channel->getIndex());
    if (index == kNew || index == kDel) {           // 需要添加进去
        if (index == kNew) {
            m_channels[channel->getFd()] = channel;
        }
        channel->setIndex(kAdd);
        update(EPOLL_CTL_ADD, channel);
    } else {
        if (channel->isNoneEvent()) {
            channel->setIndex(kDel);
            update(EPOLL_CTL_DEL, channel);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EpollPoller::removeChannel(Channel *channel) {
    m_channels.erase(channel->getFd());

    if (channel->getIndex() == kAdd) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->setIndex(kNew);
}

void EpollPoller::fillActiveChannels(int numEvents, std::vector<Channel *> &activeChannels) const {
    for (int i = 0; i < numEvents; ++i) {
        auto *channel = static_cast<Channel*>(m_events[i].data.ptr);
        channel->setRetEvents(m_events[i].events);
        activeChannels.push_back(channel);
    }
}

void EpollPoller::update(int op, Channel *channel) {
     epoll_event event{};
     memset(&event, 0, sizeof(event));
     event.events = channel->getEvents();
     event.data.fd = channel->getFd();
     event.data.ptr = channel;

     if (::epoll_ctl(m_epfd, op, channel->getFd(), &event) < 0) {
         if (op == EPOLL_CTL_DEL) {
             LOG_ERROR("epoll_ctl del error! \n");
         } else {
             LOG_FATAL("epoll_ctl add/mod error! \n");
         }
     }
}
