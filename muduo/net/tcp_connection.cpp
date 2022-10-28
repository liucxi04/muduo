//
// Created by liucxi on 2022/10/24.
//

#include "tcp_connection.h"

#include <utility>
#include <cerrno>

#include "../base/logger.h"
#include "socket.h"
#include "channel.h"
#include "event_loop.h"

static EventLoop *CheckNotNull(EventLoop *loop) {
    if (loop == nullptr) {
        LOG_FATAL("%s error! \n", __FUNCTION__);
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop *loop, std::string name, int fd,
                             InetAddress localAddr, InetAddress peerAddr)
        : m_loop(CheckNotNull(loop)), m_name(std::move(name)), m_state(kConnecting)
        , m_socket(new Socket(fd)), m_channel(new Channel(loop, fd))
        , m_localAddr(std::move(localAddr)), m_peerAddr(std::move(peerAddr))
        , m_highWaterMark(64 * 1024 * 1024) {
    // 设置相应回调函数
    m_channel->setReadCallback(std::bind(&TcpConnection::handleRead, this));
    m_channel->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    m_channel->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    m_channel->setErrorCallback(std::bind(&TcpConnection::handleError, this));
    LOG_INFO("%s run! \n", __FUNCTION__);
    m_socket->keepAlive(true);
}

TcpConnection::~TcpConnection() {
    LOG_INFO("%s run! \n", __FUNCTION__);
}

void TcpConnection::send(const std::string &buf) {
    if (m_state == kConnected) {
        // 有可能会在非本线程执行，比如有线程记录了当前 connection，并在适当时机再运行
        if (m_loop->isInLoopThread()) {
            sendInLoop(buf.c_str(), buf.size());
        } else {
            m_loop->queueInLoop(std::bind(&TcpConnection::sendInLoop, this, buf.c_str(), buf.size()));
        }
    }
}

void TcpConnection::shutdown() {
    if (m_state == kConnected) {
        setState(kDisconnecting);
        m_loop->queueInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::connectEstablished() {
    setState(kConnected);
    /**
     * @note tie 绑定，监控对象的生存状态
     */
    m_channel->tie(shared_from_this());
    m_channel->enableReading();
    // 新连接建立，执行回调
    m_connectionCallback(shared_from_this());
}

void TcpConnection::connectDestroyed() {
    if (m_state == kConnected) {
        setState(kDisconnected);
        m_channel->disableAll();
        m_connectionCallback(shared_from_this());
    }
    m_channel->remove();
}

void TcpConnection::handleRead() {
    int saveErrno = 0;
    ssize_t n = m_inputBuffer.readFd(m_channel->getFd(), saveErrno);
    if (n > 0) {
        // 已建立连接的用户有可读事件发生了
        m_messageCallback(shared_from_this(), &m_inputBuffer);
    } else if (n == 0) {
        handleClose();
    } else {
        errno = saveErrno;
        LOG_ERROR("%s error! \n", __FUNCTION__);
        handleError();
    }
}

void TcpConnection::handleWrite() {
    // 因为一般对读事件感兴趣，但不一定对写事件感兴趣
    if (m_channel->isWriteEvent()) {
        int saveErrno = 0;
        ssize_t n = m_outputBuffer.writeFd(m_channel->getFd(), saveErrno);
        if (n >= 0) {
            m_outputBuffer.retrieve(n);             // 发送了 n 个数据，可读部分进行复位
            if (m_outputBuffer.readableBytes() == 0) {  // 全部发送完成了
                m_channel->disableWriting();
                if (m_writeCompleteCallback) {
                    // runInLoop 也是可以的
                    m_loop->queueInLoop(std::bind(m_writeCompleteCallback, shared_from_this()));
                }
                // 客户端断开连接或者服务端 shutdown，关闭写
                if (m_state == kDisconnecting) {
                    shutdownInLoop();
                }
            }
        } else {
            // 写出错
            LOG_ERROR("%s error! \n", __FUNCTION__);
        }
    } else {
        // 对写事件不感兴趣
        LOG_ERROR("%s error1 \n", __FUNCTION__);
    }
}

void TcpConnection::handleClose() {
    setState(kDisconnected);
    m_channel->disableAll();                        // 取消对所有事件的订阅
    TcpConnectionPtr connPtr(shared_from_this());
    m_connectionCallback(connPtr);             // 还可以加上判空
    m_closeCallback(connPtr);
}

void TcpConnection::handleError() {
    int optVal;
    socklen_t optLen = sizeof optVal;
    int err;
    if (::getsockopt(m_channel->getFd(), SOL_SOCKET, SO_ERROR, &optVal, &optLen) < 0) {
        err = errno;
    } else {
        err = optVal;
    }
    LOG_ERROR("%s error, errno = %d", __FUNCTION__, err);
}

/**
 * @brief 发送数据
 * @details 有可能应用写的快而内核发送的慢，所以要将待发送数据写到缓冲区
 */
void TcpConnection::sendInLoop(const void *data, size_t len) {
    ssize_t wrote = 0;              // 写了多少数据
    size_t remaining = len;         // 还剩多少没法
    bool faultError = false;        // 产生了错误

    if (m_state == kDisconnected) {
        LOG_ERROR("disconnected, give up writing! \n");
        return;
    }

    // 还没有给 epoll 注册写事件呢，写缓冲区也没有待发送的数据
    if (!m_channel->isWriteEvent() && m_outputBuffer.readableBytes() == 0) {
        wrote = ::write(m_channel->getFd(), data, len);
        if (wrote >= 0) {
            remaining = len - wrote;
            // 数据发送完成
            if (remaining == 0 && m_writeCompleteCallback) {
                m_loop->queueInLoop(std::bind(m_writeCompleteCallback, shared_from_this()));
            }
        } else {
            wrote = 0;
            if (errno != EWOULDBLOCK) {                         // 非阻塞时立即返回，没有发送数据
                LOG_ERROR("%s error! \n", __FUNCTION__);
                if (errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
        }

        // 本次 write 没有发送完，剩下的写到缓冲区里，并且给 channel 设置写事件
        if (!faultError && remaining > 0) {
            size_t oldLen = m_outputBuffer.readableBytes();
            // 超出水位线
            if (oldLen + remaining >= m_highWaterMark && oldLen < m_highWaterMark && m_highWaterMarkCallback) {
                m_loop->queueInLoop(std::bind(m_highWaterMarkCallback, shared_from_this(), oldLen + remaining));
            }
            // 没有发送完的添加到缓冲区中
            m_outputBuffer.append((char*) data + wrote, remaining);
            // 注册 channel 的写事件
            if (!m_channel->isWriteEvent()) {
                m_channel->enableWriting();
            }
        }
    }
}

void TcpConnection::shutdownInLoop() {
    if (!m_channel->isWriteEvent()) {
        /// 关闭写端会触发 epoll EPOLL_HUP 事件，从而调用 TcpConnection::handleClose()
        m_socket->shutdownWrite();
    }
}

