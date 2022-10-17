//
// Created by liucxi on 2022/10/12.
//

#ifndef MUDUO_EPOLL_POLLER_H
#define MUDUO_EPOLL_POLLER_H

#include "poller.h"
#include <sys/epoll.h>

/**
 * @brief epoll 的封装类
 */
class EpollPoller : public Poller {
public:
    /**
     * @brief 构造函数
     * @param loop Epoll Poller 所属的 EventLoop
     */
    explicit EpollPoller(EventLoop *loop);

    ~EpollPoller() override;

    Time pool(int timeoutMs, std::vector<Channel *> &activeChannels) override;

    void updateChannel(Channel *channel) override;

    void removeChannel(Channel *channel) override;

private:
    static const int kInitEventListSize = 16;   // m_events 的初始大小

    /**
     * @brief 将 m_events 映射到 activeChannels；
     * @param numEvents m_events 数组的大小
     * @param activeChannels 映射的结果
     */
    void fillActiveChannels(int numEvents, std::vector<Channel *> &activeChannels) const;

    /**
     * @brief 在 epfd 上修改 channel 对应的 fd 所关注的事件
     * @param op ep_ctl 对应的操作
     * @param channel 需要修改的 channel
     */
    void update(int op, Channel *channel);
private:
    int m_epfd;                         // epoll fd
    std::vector<epoll_event> m_events;  // 保存 epoll_wait 返回的每个 fd 发生的事件
};

#endif //MUDUO_EPOLL_POLLER_H
