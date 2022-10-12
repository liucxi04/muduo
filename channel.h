//
// Created by liucxi on 2022/10/12.
//

#ifndef MUDUO_CHANNEL_H
#define MUDUO_CHANNEL_H

#include "noncopyable.h"
#include "times.h"

#include <functional>
#include <memory>

class EventLoop;

/**
 * @brief 通道类，封装类 epoll 监听的 socket fd 和 fd 感兴趣的事件，以及真正发生的事件
 */
class Channel : NonCopyable {
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Time)>;    // 只读事件

    Channel(EventLoop *loop, int fd);

    ~Channel() = default;

    void handleEvent(const Time &recvTime);

    void setReadCallback(ReadEventCallback callback) { m_readCallback = std::move(callback); };

    void setWriteCallback(EventCallback callback) { m_writeCallback = std::move(callback); };

    void setCloseCallback(EventCallback callback) { m_closeCallback = std::move(callback); };

    void setErrorCallback(EventCallback callback) { m_errorCallback = std::move(callback); };

    /// 防止 channel 被手动 remove 掉，channel 还在执行回调
    void tie(const std::shared_ptr<void> &obj);

    int getFd() const { return m_fd; };

    int getEvents() const { return m_events; };

    void setRetEvents(int retEvents) { m_retEvents = retEvents; };

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

    int getIndex() const { return m_index; };

    void setIndex(int index) { m_index = index; };

    EventLoop *ownerLoop() { return m_loop; }

    void remove();

private:
    void update();

    void handleEventWithGuard(const Time &recvTime);

private:
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop *m_loop;  // 事件循环
    const int m_fd;     // epoll 监听的 socket fd
    int m_events;       // fd 感兴趣的事件
    int m_retEvents;    // epoll 返回的实际发生的事件
    int m_index;

    std::weak_ptr<void> m_tie;
    bool m_tied;

    ReadEventCallback m_readCallback;
    EventCallback m_writeCallback;
    EventCallback m_closeCallback;
    EventCallback m_errorCallback;
};

#endif //MUDUO_CHANNEL_H
