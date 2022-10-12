//
// Created by liucxi on 2022/10/12.
//

#ifndef MUDUO_EPOLL_POLLER_H
#define MUDUO_EPOLL_POLLER_H

#include "poller.h"
#include <sys/epoll.h>

class EpollPoller : public Poller {
public:
    explicit EpollPoller(EventLoop *loop);

    ~EpollPoller() override;

    Time pool(Time timeoutMs, std::vector<Channel *> &activeChannels) override;

    void updateChannel(Channel *channel) override;

    void removeChannel(Channel *channel) override;

private:
    static const int kInitEventListSize = 16;

    void fillActiveChannels(int numEvents, std::vector<Channel> &activeChannels) const;

    void update(int op, Channel *channel);
private:
    int m_epfd;
    std::vector<epoll_event> m_events;
};

#endif //MUDUO_EPOLL_POLLER_H
