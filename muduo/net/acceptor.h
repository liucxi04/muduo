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
    void handleRead();

private:
    bool m_listening;
    EventLoop *m_loop;                              // 用户定义的 baseLoop，也就是 mainLoop;
    Socket m_acceptSocket;
    Channel m_acceptChannel;
    NewConnectionCallback m_newConnectionCallback;
};
#endif //MUDUO_ACCEPTOR_H
