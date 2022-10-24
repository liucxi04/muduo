//
// Created by liucxi on 2022/10/22.
//

#ifndef MUDUO_ACCEPTOR_H
#define MUDUO_ACCEPTOR_H

#include "../base/noncopyable.h"
#include "socket.h"
#include "channel.h"

#include <functional>

class EventLoop;

class Acceptor : NonCopyable {
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;

    Acceptor(EventLoop *loop, const InetAddress &addr, bool reusePort);

    ~Acceptor();

    void listen();

    bool isListening() const { return m_listening; }

    void setNewConnectionCallback(NewConnectionCallback cb) { m_newConnectionCallback = std::move(cb); }

private:
    /**
     * @details channel 所监听的 fd 上事件发生的回调, 即有新用户连接时进行 accept, 成功之后调用 m_newConnectionCallback,
     * 在该函数里进行轮询下发 channel
     */
    void handleRead();

private:
    bool m_listening;
    EventLoop *m_loop;                              // acceptor 运行在 loop 上, 用户定义的 baseLoop, 也就是 mainLoop;
    Socket m_acceptSocket;                          // listen fd 的封装
    Channel m_acceptChannel;                        // listen fd, fd 感兴趣的事件, epoll 返回的事件
    NewConnectionCallback m_newConnectionCallback;  // acceptor 监听并返回 client conn, 使用该回调轮询选择 subLoop
};
#endif //MUDUO_ACCEPTOR_H
