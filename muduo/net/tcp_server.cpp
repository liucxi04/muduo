//
// Created by liucxi on 2022/10/12.
//

#include "tcp_server.h"

#include <utility>
#include "../base/logger.h"

EventLoop *checkNotNull(EventLoop *loop) {
    if (loop == nullptr) {
        LOG_FATAL("%s error! \n", __FUNCTION__);
    }
    return loop;
}

TcpServer::TcpServer(EventLoop *loop, InetAddress listenAddr, TcpServer::Option op)
    : m_loop(checkNotNull(loop))
    , m_addr(std::move(listenAddr))
    , m_acceptor(new Acceptor(m_loop, listenAddr, op == kReusePort))
    , m_threadPool(new EventLoopThreadPool(loop))

    , m_started(0)
    , m_nextConnId(1)
    {
    m_acceptor->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this,
                                                   std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {

}

void TcpServer::setThreadNum(int threadNum) {
    m_threadPool->setThreadNum(threadNum);
}

void TcpServer::start() {
    if (m_started++ == 0) {     // 防止一个 TcpServer 被启动多次
        m_threadPool->start(m_threadInitCallback);
        m_loop->runInLoop(std::bind(&Acceptor::listen, m_acceptor.get()));
    }
}

void TcpServer::newConnection(int fd, const InetAddress &peerAddr) {

}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {

}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {

}

