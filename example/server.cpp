//
// Created by liucxi on 2022/10/29.
//

#include <muduo_/net/tcp_server.h>
#include <muduo_/base/logger.h>

class EchoServer {
public:
    EchoServer(EventLoop *loop, const InetAddress &addr)
        : loop_(loop), server_(loop, addr) {
        // 注册回调函数
        server_.setConnectionCallback(std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
        server_.setMessageCallback(std::bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
        // 设置合适的线程
        server_.setThreadNum(3);
    }

    void start() {
        server_.start();
    }
private:
    void onConnection(const TcpConnectionPtr &conn) {
        if (conn->isConnected()) {
            LOG_INFO("connect up! \n");
        } else {
            LOG_INFO("connect down! \n");
        }
    }

    void onMessage(const TcpConnectionPtr &conn, Buffer *buf) {
        std::string msg = buf->retrieveAllAsString();
        conn->send(msg);
        conn->shutdown();           // 关闭写端，epoll 相应 EPOLLHUP， 然后执行 closeCallback
    }
private:
    EventLoop *loop_;
    TcpServer server_;
};
int main() {
    EventLoop loop;
    InetAddress addr("127.0.0.1", 12345);
    EchoServer server(&loop, addr);

    server.start();
    loop.loop();
    return 0;
}