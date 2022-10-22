//
// Created by liucxi on 2022/10/22.
//

#ifndef MUDUO_SOCKET_H
#define MUDUO_SOCKET_H

class InetAddress;

class Socket {
public:
    explicit Socket(int fd);

    ~Socket();

    int getFd() const { return m_fd; }

    void bindAddr(const InetAddress &localAddr);

    void listen();

    int accept(InetAddress &peerAddr);

    void shutdownWrite();

    void setTcpNoDelay(bool on);

    void setReuseAddr(bool on);

    void setReusePort(bool on);

    void keepAlive(bool on);

private:
    int m_fd;
};


#endif //MUDUO_SOCKET_H
