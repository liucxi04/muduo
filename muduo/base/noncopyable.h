//
// Created by liucxi on 2022/10/12.
//

#ifndef MUDUO_NONCOPYABLE_H
#define MUDUO_NONCOPYABLE_H

/**
 * @brief NoCopyable 被继承以后，派生类可以正常的构造和析构，但是不能被拷贝和移动
 */
class NonCopyable {
public:
    NonCopyable(const NonCopyable &) = delete;

    NonCopyable& operator=(const NonCopyable &) = default;

    NonCopyable(NonCopyable &&) = delete;

    NonCopyable& operator=(NonCopyable &&) = default;

protected:
    NonCopyable() = default;

    ~NonCopyable() = default;
};

#endif //MUDUO_NONCOPYABLE_H
