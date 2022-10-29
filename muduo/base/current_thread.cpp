//
// Created by i on 2022/10/29.
//

#include "current_thread.h"

#include <sys/syscall.h>
#include <unistd.h>

namespace CurrentThread {

    thread_local int t_cachedTid = 0;

    /**
     * @brief 获得当前线程的 tid
     */
     void cachedTid() {
        if (t_cachedTid == 0) {
            t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
        }
     }
}

