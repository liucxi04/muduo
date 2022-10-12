//
// Created by liucxi on 2022/10/12.
//

#include "poller.h"

#include <cstdlib>

Poller *Poller::NewDefaultPoller(EventLoop *loop) {
    if (::getenv("MUDUO_USE_POOL")) {
        return nullptr;
    } else {
        return nullptr;
    }
}