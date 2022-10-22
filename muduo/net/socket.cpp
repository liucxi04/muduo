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
    sockaddr_in addr{};
    socklen_t len = 0;
    memset(&addr, 0 , sizeof(addr));
    int fd = ::accept(m_fd, (sockaddr*) &addr, &len);
    if (fd > 0) {
        peerAddr.setAddr(addr);
    }
    return fd;
}

void Socket::shutdownWrite() {
    ::shutdown(m_fd, SHUT_WR);
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

