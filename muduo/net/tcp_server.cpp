//
// Created by liucxi on 2022/10/12.
//

#include "tcp_server.h"

#include <utility>
#include "../base/logger.h"

static EventLoop *CheckNotNull(EventLoop *loop) {
    if (loop == nullptr) {
        LOG_FATAL("%s error! \n", __FUNCTION__);
    }
    return loop;
}

/**
 * @brief TcpServer 的构造函数
 * @details
 * 1. m_acceptor, 属于 mainLoop 的 Acceptor, 用来监听新连接并将连接以 Connection 的形式下发到 subLoop.
 *    在 m_acceptor 构造时, 会创建一个 listen fd, 并将 fd 封装成 Socket 和 Channel, 并为 Channel 绑定读事件回调.
 *    Channel 绑定的读事件回调为 Acceptor::handleRead, 该事件被触发说明有新连接到来, 回调一开始会进行 ::accept,
 *    然后调用 TcpServer 下发的 newConnectionCallback, 将 connect fd 封装成 Connection 通过轮询的方式分发给 subLoop.
 *    m_acceptor 调用 Acceptor::listen 开启监听 ::listen, 并将 Channel 通过 enableReading() 添加到 mainLoop 中.
 *    TcpServer 会通过 TcpServer::start 启动 Acceptor::listen 从而启动 m_acceptor 进行 listen, accept 和 分发.
 * 2.
 */
TcpServer::TcpServer(EventLoop *loop, InetAddress listenAddr, TcpServer::Option op)
    : m_started(0)
    , m_loop(CheckNotNull(loop))
    , m_addr(std::move(listenAddr))
    , m_acceptor(new Acceptor(m_loop, m_addr, op == kReusePort))
    , m_threadPool(new EventLoopThreadPool(loop))

    , m_nextConnId(1)
    {
    // 下发给 acceptor 的回调, 当有新用户连接时会被调用
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
        m_threadPool->start(m_threadInitCallback);      // 启动底层的线程池
        m_loop->runInLoop(std::bind(&Acceptor::listen, m_acceptor.get()));
    }
}

/**
 * @brief
 * 通过轮询算法选择一个 subLoop, 通过每个 subLoop 上所监听的 wakeupFd 唤醒该 loop, 然后将 fd 封装成 channel 传递给该 loop,
 * 在此之前需要为 channel 绑定相应的事件回调
 * m_connectionCallback 会在创建完 Connection 对象, 调用 ioLoop->runInLoop() 时在 TcpConnection::connectEstablished
 * 里被调用
 */
void TcpServer::newConnection(int fd, const InetAddress &peerAddr) {

}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {

}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {

}

