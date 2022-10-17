//
// Created by liucxi on 2022/10/12.
//

#ifndef MUDUO_CHANNEL_H
#define MUDUO_CHANNEL_H

#include "../base/noncopyable.h"

#include <functional>
#include <memory>

class EventLoop;

/**
 * @brief 通道类，封装类 epoll 监听的 socket fd 和 fd 感兴趣的事件，以及真正发生的事件
 */
class Channel : NonCopyable {
public:
    using EventCallback = std::function<void()>;        // 事件回调函数

    /**
     * @brief 构造函数
     * @param loop Channel 所属的 EventLoop
     * @param fd Channel 所负责的 socket fd，注意它不拥有该 fd，也不会在析构时负责关闭
     */
    Channel(EventLoop *loop, int fd);

    /**
     * @brief 默认的析构函数
     * @note 一个 Channel 只属于一个 EventLoop，析构时也应该在其所属的 EventLoop 对应的线程里析构
     */
    ~Channel() = default;

    /**
     * @brief 根据发生的具体事件调用具体的回调函数
     */
    void handleEvent();

    void setReadCallback(EventCallback callback) { m_readCallback = std::move(callback); };

    void setWriteCallback(EventCallback callback) { m_writeCallback = std::move(callback); };

    void setCloseCallback(EventCallback callback) { m_closeCallback = std::move(callback); };

    void setErrorCallback(EventCallback callback) { m_errorCallback = std::move(callback); };

    /***
     * @details 防止 channel 被手动 remove 掉，channel 还在执行回调
     * @param obj 所观察的对象
     */
    void tie(const std::shared_ptr<void> &obj);

    int getFd() const { return m_fd; };

    int getEvents() const { return m_events; };

    void setRetEvents(int retEvents) { m_retEvents = retEvents; };

    int getIndex() const { return m_index; };

    void setIndex(int index) { m_index = index; };

    EventLoop *getOwnerLoop() { return m_loop; }

    bool isNoneEvent() const { return m_events == kNoneEvent; };

    bool isReadEvent() const { return m_events & kReadEvent; };

    bool isWriteEvent() const { return m_events & kWriteEvent; };

    void enableReading() {
        m_events |= kReadEvent;
        update();
    };

    void disableReading() {
        m_events &= ~kReadEvent;
        update();
    };

    void enableWriting() {
        m_events |= kWriteEvent;
        update();
    };

    void disableWriting() {
        m_events &= ~kWriteEvent;
        update();
    };

    void disableAll() { m_events = kNoneEvent; };

    /**
     * @brief 从 poller 中移除该 channel，即 epoll 不在监听 该 socket fd
     */
    void remove();

private:
    /**
     * @brief 通知 poller 更新感兴趣的事件
     */
    void update();

    void handleEventWithGuard();

private:

    /// 关心的事件枚举，无事件，读事件，写事件
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    /// 核心的数据成员
    EventLoop *m_loop;  // 所属的 EventLoop
    const int m_fd;     // 监听的 socket fd
    int m_events;       // fd 感兴趣的事件
    int m_retEvents;    // epoll_wait 返回的实际发生的事件
    int m_index;        // 该 channel 在 epoll 中的状态，见 epoll_poller.cpp

    /// 防止 channel 被手动 remove 掉，channel 还在执行回调
    /// 实现了一个跨线程的生存状态的监听
    std::weak_ptr<void> m_tie;
    bool m_tied;

    /// 具体的事件对应的回调函数
    EventCallback m_readCallback;
    EventCallback m_writeCallback;
    EventCallback m_closeCallback;
    EventCallback m_errorCallback;

    /**
     * @note Channel 实际所属一个线程，无多线程操作，所以不需要加锁
     */
     // std::mutex m_mutex;
};

#endif //MUDUO_CHANNEL_H
