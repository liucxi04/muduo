//
// Created by liucxi on 2022/10/12.
//

#ifndef MUDUO_TCP_SERVER_H
#define MUDUO_TCP_SERVER_H

#include "event_loop.h"
#include "acceptor.h"
#include "address.h"
#include "callback.h"
#include "event_loop_thread_loop.h"
#include "../base/noncopyable.h"

#include <atomic>
#include <unordered_map>

class TcpServer : NonCopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    enum Option {
        kNoReusePort,
        kReusePort,
    };

    TcpServer(EventLoop *loop, InetAddress listenAddr, Option op = kNoReusePort);

    ~TcpServer();

    /**
     * @brief 设置底层 subLoop 的个数
     * @param threadNum 子线程数, subLoop 的数量
     */
    void setThreadNum(int threadNum);

    /**
     * @brief 开启服务器监听
     */
    void start();

    void setConnectionCallback(ConnectionCallback cb) { m_connectionCallback = std::move(cb); }

    void setMessageCallback(MessageCallback cb) { m_messageCallback = std::move(cb); }

    void setWriteCompleteCallback(WriteCompleteCallback cb) { m_writeCompleteCallback = std::move(cb); }

    void setThreadInitCallback(ThreadInitCallback cb) { m_threadInitCallback = std::move(cb); }

private:
    /**
     * @brief 当有新用户连接时，会调用该函数，tcpServer 会将该函数下发至 Acceptor
     * @param fd socket accept 返回的用户端连接
     * @param peerAddr 客户端地址
     */
    void newConnection(int fd, const InetAddress &peerAddr);

    void removeConnection(const TcpConnectionPtr &conn);

    void removeConnectionInLoop(const TcpConnectionPtr &conn);

private:
    EventLoop *m_loop;
    InetAddress m_addr;
    std::unique_ptr<Acceptor> m_acceptor;                   // 运行在 mainLoop 里的 acceptor
    std::shared_ptr<EventLoopThreadPool> m_threadPool;

    ConnectionCallback m_connectionCallback;                // 有新连接时的回调
    MessageCallback m_messageCallback;                      // 有读写消息时的回调
    WriteCompleteCallback m_writeCompleteCallback;          // 消息发送完成以后的回调
    ThreadInitCallback m_threadInitCallback;                // 线程创建完成后初始化 loop 的函数

    std::atomic_int m_started;
    int m_nextConnId;
    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;
    ConnectionMap m_connects;
};

#endif //MUDUO_TCP_SERVER_H
