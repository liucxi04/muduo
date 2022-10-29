//
// Created by liucxi on 2022/10/12.
//

#ifndef MUDUO_TCP_SERVER_H
#define MUDUO_TCP_SERVER_H

/// 引入需要的头文件而不是类的前置声明, 简化用户的使用
#include "event_loop.h"
#include "acceptor.h"
#include "address.h"
#include "callback.h"
#include "event_loop_thread_pool.h"
#include "tcp_connection.h"
#include "buffer.h"
#include "../base/noncopyable.h"

#include <atomic>
#include <unordered_map>

class TcpServer : NonCopyable {
public:
    /**
     * @brief 线程初始化回调函数，在新线程创建 EventLoop 之后调用
     */
    using ThreadInitCallback = std::function<void(EventLoop *)>;
    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

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
    std::atomic_int m_started;                              // tcp server 是否启动
    EventLoop *m_loop;                                      // baseLoop, mainLoop, 用户定义的 loop
    InetAddress m_addr;                                     // 主机的 ip 和 port
    std::unique_ptr<Acceptor> m_acceptor;                   // 运行在 mainLoop 里的 acceptor
    std::shared_ptr<EventLoopThreadPool> m_threadPool;      // 线程池, 包含多个线程, 每个线程里运行一个 EventLoop, one loop pre thread

    ConnectionCallback m_connectionCallback;                // 有新连接时的回调
    MessageCallback m_messageCallback;                      // 有读写消息时的回调
    WriteCompleteCallback m_writeCompleteCallback;          // 消息发送完成以后的回调
    ThreadInitCallback m_threadInitCallback;                // 线程创建完成后初始化 loop 的函数

    int m_nextConnId;
    ConnectionMap m_connects;                               // 保存所有的连接
};

#endif //MUDUO_TCP_SERVER_H