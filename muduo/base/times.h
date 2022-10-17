//
// Created by liucxi on 2022/10/12.
//

#ifndef MUDUO_TIMES_H
#define MUDUO_TIMES_H

#include <cstdint>
#include <string>

/**
 * @brief 时间类
 */
class Time {
public:
    explicit Time(int64_t ms = 0);

    static Time now();

    std::string toString() const;

private:
    int64_t m_ms;
};

#endif //MUDUO_TIMES_H
