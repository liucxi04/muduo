//
// Created by liucxi on 2022/10/13.
//

#ifndef MUDUO_CURRENT_THREAD_H
#define MUDUO_CURRENT_THREAD_H

namespace CurrentThread {
    extern thread_local int t_cachedTid;

    void cachedTid();

    inline int tid() {
        if (__builtin_expect(t_cachedTid == 0, 0)) {
            cachedTid();
        }
        return t_cachedTid;
    }
}

#endif //MUDUO_CURRENT_THREAD_H
