//
// Created by liucxi on 2022/10/12.
//

#include "poller.h"

#include "../channel.h"

Poller::Poller(EventLoop *loop)
        : m_loop(loop) {
}

bool Poller::hasChannel(Channel *channel) const {
    auto it = m_channels.find(channel->getFd());
    return it != m_channels.end() && it->second == channel;
}
