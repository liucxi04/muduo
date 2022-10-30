# muduo

## muduo 库编译
```shell
sudo ./autobuild

# 库文件目录
cd /usr/include/muduo_
# so 文件目录
cd /usr/lib
```

## 示例代码编译运行
```shell
cd example
make clean
make
./server
```

## muduo 库的核心模块

### 1. Channel
一个 socket fd；(listen fd --> Acceptor、conn fd --> TcpConnection)

该 fd 所感兴趣的事件，即注册到 Poller 上的需要监听的事件；

Poller 返回的实际发生的事件；

不同类型的回调函数：读、写、关闭链接、发生错误

**所属的事件循环 EventLoop**

### 2. Poller、EpollPoller
该 Poller 所管理的所有 socket fd，std::unordered_map<int, Channel*>

**所属的事件循环 EventLoop**

### 3. EventLoop  --> 相当于Reactor
wakeupFd，用来唤醒该 EventLoop

std::vector<Channel*>，所管理的所有 Channel

std::unique_ptr<Poller>，所管理的 Poller

Channel 和 Poller 通过 EventLoop 间接操作对方

### 4. Thread、EventLoopThread、EventLoopThreadPool
getNextLoop()：通过轮询选择下一个 subLoop

setThreadNum()：one loop per thread

### 5. Socket、Acceptor
对 socket fd 的基本封装

对 listen fd 的相关操作的封装：socket bind listen 读事件到来进行 accept

### 6. Buffer
非阻塞 IO 都需要设置缓冲区

### 7. TcpConnection
一个连接成功的客户端对应一个 TcpConnection

包含 Socket、Channel 和一系列的回调，还有缓冲区

### 8. TcpServer
Acceptor、EventLoopThreadPool、







