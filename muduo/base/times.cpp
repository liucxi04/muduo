//
// Created by liucxi on 2022/10/12.
//

#include "times.h"

Time::Time(int64_t ms) : m_ms(ms) { }

Time Time::now() {
    return Time(time(nullptr));
}

std::string Time::toString() const {
    char buf[128] = {0};
    tm *tm_time = localtime(&m_ms);
    snprintf(buf, 128, "%4d %02d %02d - %02d:%02d:%02d",
             tm_time->tm_year + 1900,
             tm_time->tm_mon + 1,
             tm_time->tm_mday,
             tm_time->tm_hour,
             tm_time->tm_min,
             tm_time->tm_sec);
    return buf;
}