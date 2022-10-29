//
// Created by liucxi on 2022/10/12.
//

#include "tcp_server.h"

#include <utility>
#include <cstring>
#include "tcp_connection.h"
#include "event_loop_thread_pool.h"
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
    , m_nextConnId(1) {
    // 下发给 acceptor 的回调, 当有新用户连接时会被调用
    m_acceptor->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this,
                                                   std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {
    for (auto &item : m_connects) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }
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
 * 在此之前需要为 channel 绑定相应的事件回调. m_connectionCallback 会在创建完 Connection 对象, 调用 ioLoop->runInLoop()
 * 时在 TcpConnection::connectEstablished 里被调用
 */
void TcpServer::newConnection(int fd, const InetAddress &peerAddr) {
    EventLoop *ioLoop = m_threadPool->getNextLoop();                // 轮询选择一个 subLoop
    char buf[64] = {0};
    snprintf(buf, sizeof buf, "-%s#%d", m_addr.toString().c_str(), m_nextConnId);
    ++m_nextConnId;             // 连接数量

    // 通过 socket fd 获得其绑定的本机的 ip 地址和端口号
    sockaddr_in local{};
    socklen_t len = sizeof local;
    memset(&local, 0, sizeof(local));
    ::getsockname(fd, (sockaddr *) &local, &len);
    InetAddress localAddr(local);

    // 根据连接成功的 socket fd，创建 TcpConnection 对象
    TcpConnectionPtr conn(new TcpConnection(ioLoop, buf, fd, localAddr, peerAddr));
    m_connects[buf] = conn;
    conn->setConnectionCallback(m_connectionCallback);
    conn->setMessageCallback(m_messageCallback);
    conn->setWriteCompleteCallback(m_writeCompleteCallback);
    // 设置了如何关闭连接的回调
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));

    // 直接调用 TcpConnection::connectEstablished
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
    m_loop->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {
    m_connects.erase(conn->getName());
    EventLoop *ioLoop = conn->getLoop();
    ioLoop->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}