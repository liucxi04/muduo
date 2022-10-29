//
// Created by liucxi on 2022/10/24.
//

#ifndef MUDUO_TCP_CONNECTION_H
#define MUDUO_TCP_CONNECTION_H

#include "../base/noncopyable.h"
#include "address.h"
#include "callback.h"
#include "buffer.h"

#include <memory>
#include <atomic>

class Channel;

class EventLoop;

class Socket;

/**
 * @brief 新连接的 socket fd 封装, 传递给 subLoop
 */
class TcpConnection : NonCopyable, std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(EventLoop *loop, std::string name, int fd,
                  InetAddress localAddr, InetAddress peerAddr);

    ~TcpConnection();

    /**
     * @brief 用来发送数据
     */
    void send(const std::string &buf);

    /**
     * @brief 关闭当前连接
     */
    void shutdown();

    bool isConnected() const { return m_state == kConnected; }

    bool isDisConnected() const { return m_state == kDisconnected; }

    const std::string &getName() const { return m_name; }

    EventLoop *getLoop() const { return m_loop; }

    void setConnectionCallback(const ConnectionCallback &cb) { m_connectionCallback = cb; }

    void setMessageCallback(const MessageCallback &cb) { m_messageCallback = cb; }

    void setWriteCompleteCallback(const WriteCompleteCallback &cb) { m_writeCompleteCallback = cb; }

    void setCloseCallback(const CloseCallback &cb) { m_closeCallback = cb; }

    void setHighWaterMarkCallback(const HighWaterMarkCallback &cb, size_t highWaterMark) {
        m_highWaterMarkCallback = cb;
        m_highWaterMark = highWaterMark;
    }

    /**
     * @brief 连接建立
     */
    void connectEstablished();

    /**
     * @brief 连接销毁
     */
    void connectDestroyed();

private:
    enum StateE {
        kConnecting,
        kConnected,
        kDisconnecting,
        kDisconnected,
    };

    void setState(StateE state) {
        m_state = state;
    }

    void handleRead();

    void handleWrite();

    void handleClose();

    void handleError();

    /**
     * @brief send 函数注册到 EventLoop 内的回调
     */
    void sendInLoop(const void *message, size_t len);

    /**
     * @brief shutdown 函数注册到 EventLoop 内的回调
     */
    void shutdownInLoop();

private:
    EventLoop *m_loop;          // 通过轮询选出的 subLoop
    const std::string m_name;   // 连接的名字，用在 ConnectionMap 上
    std::atomic_int m_state;    // 连接的状态，有四个枚举值

    std::unique_ptr<Socket> m_socket;
    std::unique_ptr<Channel> m_channel;
    const InetAddress m_localAddr;
    const InetAddress m_peerAddr;

    ConnectionCallback m_connectionCallback;
    MessageCallback m_messageCallback;
    WriteCompleteCallback m_writeCompleteCallback;
    CloseCallback m_closeCallback;
    HighWaterMarkCallback m_highWaterMarkCallback;
    size_t m_highWaterMark;

    Buffer m_inputBuffer;       // 接收数据的缓冲区
    Buffer m_outputBuffer;      // 发送数据的缓冲区
};

#endif //MUDUO_TCP_CONNECTION_H