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

    void send(const std::string &buf);

    void shutdown();

    void setConnectionCallback(const ConnectionCallback &cb) { m_connectionCallback = cb; }

    void setMessageCallback(const MessageCallback &cb) { m_messageCallback = cb; }

    void setWriteCompleteCallback(const WriteCompleteCallback &cb) { m_writeCompleteCallback = cb; }

    void setCloseCallback(const CloseCallback &cb) { m_closeCallback = cb; }

    void setHighWaterMarkCallback(const HighWaterMarkCallback &cb, size_t highWaterMark) {
        m_highWaterMarkCallback = cb;
        m_highWaterMark = highWaterMark;
    }

    void connectEstablished();

    void connectDestroyed();

private:
    enum StateE {
        kConnecting,
        kConnected,
        kDisconnecting,
        kDisconnected,
    };

    void handleRead();

    void handleWrite();

    void handleClose();

    void handleError();

    void sendInLoop(const void *message, size_t len);

    void shutdownInLoop();

    void setState(StateE state) {
        m_state = state;
    }

private:
    EventLoop *m_loop;          // subLoop
    const std::string m_name;
    std::atomic_int m_state;
    bool m_reading;

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

    Buffer m_inputBuffer;
    Buffer m_outputBuffer;
};

#endif //MUDUO_TCP_CONNECTION_H
