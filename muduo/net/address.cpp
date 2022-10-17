//
// Created by liucxi on 2022/10/12.
//

#include "address.h"

#include <cstring>
#include <arpa/inet.h>

InetAddress::InetAddress(std::string ip, uint16_t port)
    : m_ip(std::move(ip)), m_port(port) {
    memset(&m_addr, 0 , sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = inet_addr(m_ip.c_str());
    m_addr.sin_port = htons(m_port);
}

std::string InetAddress::getIP() const {
    return m_ip;
}

uint16_t InetAddress::getPort() const {
    return m_port;
}

const sockaddr_in *InetAddress::getAddr() const {
    return &m_addr;
}