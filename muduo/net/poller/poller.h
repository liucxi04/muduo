//
// Created by liucxi on 2022/10/12.
//

#ifndef MUDUO_POLLER_H
#define MUDUO_POLLER_H

#include "../../base/noncopyable.h"
#include "../../base/times.h"

#include <vector>
#include <unordered_map>

class Channel;

class EventLoop;

/**
 * @brief 抽象类，可以派生出 POLL 和 EPOLL
 */
class Poller : NonCopyable {
public:
    /**
     * @brief 构造函数
     * @param loop Poller 所属的 EvenLoop
     */
    explicit Poller(EventLoop *loop);

    virtual ~Poller() = default;

    /**
     * @brief IO 复用的核心，可以实现为 poll() 或者 epoll_wait()
     * @param timeoutMs 超时事件
     * @param activeChannels 有事件发生的 Channel
     * @return 返回结果的时间
     */
    virtual Time pool(int timeoutMs, std::vector<Channel *> &activeChannels) = 0;

    virtual void updateChannel(Channel *channel) = 0;

    virtual void removeChannel(Channel *channel) = 0;

    bool hasChannel(Channel *channel) const;

    /**
     * @brief 返回一个默认的 IO 复用处理实现对象
     * @note 实现在单独的文件中，避免了基类所在文件包含派生类所在文件
     */
    static Poller *NewDefaultPoller(EventLoop *loop);

protected:
    std::unordered_map<int, Channel *> m_channels;      // Poller 所关注的 Channel，
    // EventLoop 里也有 ChannelList，为什么不用这个？因为 ChannelList 里的 Channel 可能被逻辑删除了
private:
    EventLoop *m_loop;          // Poller 所属的 EvenLoop
};

#endif //MUDUO_POLLER_H
