//
// Created by liucxi on 2022/10/13.
//

#ifndef MUDUO_CURRENT_THREAD_H
#define MUDUO_CURRENT_THREAD_H

#include <sys/syscall.h>
#include <unistd.h>

thread_local int t_cachedTid = 0;

/**
 * @brief 获得当前线程的 tid
 */
inline int currentTid() {
    if (t_cachedTid == 0) {
        t_cachedTid = static_cast<int>(::syscall(SYS_gettid));
    }
    return t_cachedTid;
}
#endif //MUDUO_CURRENT_THREAD_H
