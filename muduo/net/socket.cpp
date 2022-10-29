//
// Created by liucxi on 2022/10/22.
//
#include "socket.h"
#include "address.h"
#include "../base/logger.h"

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <cstring>

Socket::Socket(int fd) : m_fd(fd) {
}

Socket::~Socket() {
    ::close(m_fd);
}

void Socket::bindAddr(const InetAddress &localAddr) {
    if (::bind(m_fd, (sockaddr *) localAddr.getAddr(), sizeof(sockaddr_in)) != 0) {
        LOG_FATAL("%s error! \n", __FUNCTION__);
    }
}

void Socket::listen() {
    if (::listen(m_fd, 1024) != 0) {
        LOG_FATAL("%s error! \n", __FUNCTION__);
    }
}

int Socket::accept(InetAddress &peerAddr) {
    /**
     * @note 第一版有错误
     * 1. accept 参数不核发
     * 2. 没有设置非阻塞
     */
    sockaddr_in addr{};
    socklen_t len = sizeof addr;                // 1.
    memset(&addr, 0 , sizeof(addr));
    int fd = ::accept4(m_fd, (sockaddr*) &addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);    // 2.
    if (fd >= 0) {
        peerAddr.setAddr(addr);
    }
    return fd;
}

void Socket::shutdownWrite() {
    if (::shutdown(m_fd, SHUT_WR) < 0) {
        LOG_ERROR("%s error! \n", __FUNCTION__ );
    }
}

void Socket::setTcpNoDelay(bool on) {
    int opt = on ? 1 : 0;
    ::setsockopt(m_fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
}

void Socket::setReuseAddr(bool on) {
    int opt = on ? 1 : 0;
    ::setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

void Socket::setReusePort(bool on) {
    int opt = on ? 1 : 0;
    ::setsockopt(m_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
}

void Socket::keepAlive(bool on) {
    int opt = on ? 1 : 0;
    ::setsockopt(m_fd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));
}

