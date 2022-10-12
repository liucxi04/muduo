//
// Created by liucxi on 2022/10/12.
//

#ifndef MUDUO_POLLER_H
#define MUDUO_POLLER_H

#include "noncopyable.h"
#include "times.h"

#include <vector>
#include <unordered_map>

class Channel;

class EventLoop;

class Poller : NonCopyable {
public:
    explicit Poller(EventLoop *loop);

    virtual ~Poller() = default;

    virtual Time pool(Time timeoutMs, std::vector<Channel *> &activeChannels) = 0;

    virtual void updateChannel(Channel *channel) = 0;

    virtual void removeChannel(Channel *channel) = 0;

    bool hasChannel(Channel *channel) const;

    /**
     * @note 实现在单独的文件中
     */
    static Poller *NewDefaultPoller(EventLoop *loop);

protected:
    std::unordered_map<int, Channel *> m_channels;
private:
    EventLoop *m_loop;
};

#endif //MUDUO_POLLER_H
