//
// Created by liucxi on 2022/10/22.
//

#include "acceptor.h"

#include "../base/logger.h"
#include "address.h"

#include <sys/socket.h>
#include <unistd.h>

static int createNonBlocking() {
    int fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (fd < 0) {
        LOG_FATAL("%s error! \n", __FUNCTION__);
    }
    return fd;
}

Acceptor::Acceptor(EventLoop *loop, const InetAddress &addr, bool reusePort)
    : m_listening(false)
    , m_loop(loop)
    , m_acceptSocket(createNonBlocking())               // 创建 socket
    , m_acceptChannel(loop, m_acceptSocket.getFd()) {
    m_acceptSocket.setReuseAddr(true);
    m_acceptSocket.setReusePort(reusePort);
    m_acceptSocket.bindAddr(addr);                // 绑定 socket
    m_acceptChannel.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
    m_acceptChannel.disableAll();
    m_acceptChannel.remove();
}

void Acceptor::listen() {
    m_listening = true;
    m_acceptSocket.listen();                            // 开始监听
    /**
     * @note @details 在这里已经将 m_acceptChannel 加入到了他所属的 loop
     */
    m_acceptChannel.enableReading();
}

/**
 * @note listen fd 有事件发生了, 即有新用户连接到来
 */
void Acceptor::handleRead() {
    InetAddress peerAddr;
    int fd = m_acceptSocket.accept(peerAddr);
    if (fd > 0) {
        if (m_newConnectionCallback) {
            m_newConnectionCallback(fd, peerAddr);
        } else {
            ::close(fd);
        }
    } else {
        LOG_ERROR("%s error!\\n", __FUNCTION__);
    }
}

