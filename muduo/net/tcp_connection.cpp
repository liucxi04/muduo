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

static EventLoop *checkNotNull(EventLoop *loop) {
    if (loop == nullptr) {
        LOG_FATAL("%s error! \n", __FUNCTION__);
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop *loop, std::string name, int fd,
                             InetAddress localAddr, InetAddress peerAddr)
        : m_loop(checkNotNull(loop)), m_name(std::move(name)), m_state(kConnecting), m_reading(true),
          m_socket(new Socket(fd)), m_channel(new Channel(loop, fd)), m_localAddr(std::move(localAddr)),
          m_peerAddr(std::move(peerAddr)), m_highWaterMark(64 * 1024 * 1024) {
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

void TcpConnection::shutdown() {
    if (m_state == kConnected) {
        setState(kDisconnecting);
        m_loop->queueInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::connectEstablished() {
    setState(kConnected);
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

void TcpConnection::send(const std::string &buf) {
    if (m_state == kConnected) {
        if (m_loop->isInLoopThread()) {
            sendInLoop(buf.c_str(), buf.size());
        } else {
            m_loop->runInLoop(std::bind(&TcpConnection::sendInLoop, this, buf.c_str(), buf.size()));
        }
    }
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
    if (m_channel->isWriteEvent()) {
        int saveErrno = 0;
        ssize_t n = m_outputBuffer.writeFd(m_channel->getFd(), saveErrno);
        if (n > 0) {
            m_outputBuffer.retrieve(n);
            if (m_outputBuffer.readableBytes() == 0) {
                m_channel->disableWriting();
                if (m_writeCompleteCallback) {
                    m_loop->queueInLoop(std::bind(m_writeCompleteCallback, shared_from_this()));
                }
                if (m_state == kDisconnecting) {
                    shutdownInLoop();
                }
            }
        } else {
            LOG_ERROR("%s error! \n", __FUNCTION__);
        }
    } else {
        // 不可写
        LOG_ERROR("%s error1 \n", __FUNCTION__);
    }
}

void TcpConnection::handleClose() {
    setState(kDisconnected);
    m_channel->disableAll();
    TcpConnectionPtr connPtr(shared_from_this());
    m_connectionCallback(connPtr);
    m_closeCallback(connPtr);
}

void TcpConnection::handleError() {
    int optVal;
    socklen_t optLen = sizeof optVal;
    int err = 0;
    if (::getsockopt(m_channel->getFd(), SOL_SOCKET, SO_ERROR, &optVal, &optLen) < 0) {
        err = errno;
        LOG_ERROR("%s errno = %d", __FUNCTION__, optVal);
    } else {
        err = optVal;
    }
    LOG_ERROR("%s error, errno = %d", __FUNCTION__, err);
}

void TcpConnection::sendInLoop(const void *message, size_t len) {
    ssize_t wrote = 0;
    size_t remaining = len;
    bool faultError = false;

    if (m_state == kDisconnected) {
        LOG_ERROR("disconnected, give up writing! \n");
        return;
    }

    if (!m_channel->isWriteEvent() && m_outputBuffer.readableBytes() == 0) {
        wrote = ::write(m_channel->getFd(), message, len);
        if (wrote >= 0) {
            remaining = len - wrote;
            // 数据发送完成
            if (remaining == 0 && m_writeCompleteCallback) {
                m_loop->queueInLoop(std::bind(m_writeCompleteCallback, shared_from_this()));
            }
        } else {
            wrote = 0;
            if (errno != EWOULDBLOCK) {
                LOG_ERROR("%s error! \n", __FUNCTION__);
                if (errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
        }

        // 本次 write 没有发送完
        if (!faultError && remaining > 0) {
            size_t oldLen = m_outputBuffer.readableBytes();
            if (oldLen + remaining >= m_highWaterMark && oldLen < m_highWaterMark && m_highWaterMarkCallback) {
                m_loop->queueInLoop(std::bind(m_highWaterMarkCallback, shared_from_this(), oldLen + remaining));
            }
            m_outputBuffer.append((char*) message + wrote, remaining);
            if (!m_channel->isWriteEvent()) {
                m_channel->enableWriting();
            }
        }
    }
}

void TcpConnection::shutdownInLoop() {
    if (!m_channel->isWriteEvent()) {
        m_socket->shutdownWrite();
    }
}

