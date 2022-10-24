//
// Created by liucxi on 2022/10/24.
//

#ifndef MUDUO_CALLBACK_H
#define MUDUO_CALLBACK_H

#include <memory>
#include <functional>
#include "../base/times.h"

class Buffer;

class TcpConnection;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;

using CloseCallback = std::function<void(const TcpConnectionPtr&)>;

using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;

using MessageCallback = std::function<void(const TcpConnectionPtr&, Buffer*, Time)>;

#endif //MUDUO_CALLBACK_H
