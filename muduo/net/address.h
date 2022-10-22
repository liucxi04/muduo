//
// Created by liucxi on 2022/10/12.
//

#ifndef MUDUO_ADDRESS_H
#define MUDUO_ADDRESS_H

#include <netinet/in.h>
#include <string>

/**
 * @brief sockaddr_in 的封装类
 */
class InetAddress {
public:
    explicit InetAddress(std::string ip = "", uint16_t port = 0);

    std::string getIP() const;

    uint16_t getPort() const;

    const sockaddr_in *getAddr() const;

    void setAddr(const sockaddr_in &addr);

private:
    std::string m_ip;
    uint16_t m_port;
    sockaddr_in m_addr{};
};

#endif //MUDUO_ADDRESS_H
