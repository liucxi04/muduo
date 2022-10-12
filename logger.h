//
// Created by liucxi on 2022/10/12.
//

#ifndef MUDUO_LOGGER_H
#define MUDUO_LOGGER_H

#include <string>

#include "noncopyable.h"

/**
 * @brief 定义工具宏，方便用户使用日志功能
 */
/// 控制 DEBUG 信息的输出
#ifdef MUDUO_DEBUG
#define LOG_DEBUG(logMessageFormat, ...)                            \
    do {                                                            \
        Logger &logger = Logger::GetInstance();                     \
        logger.setLevel(LogLevel::DEBUG);                           \
        char message[1024];                                         \
        snprintf(message, 1024, logMessageFormat, ##__VA_ARGS__);   \
        logger.log(message);                                        \
    } while (0);
#else
#define LOG_DEBUG(logMessageFormat, ...)
#endif

#define LOG_INFO(logMessageFormat, ...)                             \
    do {                                                            \
        Logger &logger = Logger::GetInstance();                     \
        logger.setLevel(LogLevel::INFO);                            \
        char message[1024];                                         \
        snprintf(message, 1024, logMessageFormat, ##__VA_ARGS__);   \
        logger.log(message);                                        \
    } while (0);

#define LOG_ERROR(logMessageFormat, ...)                            \
    do {                                                            \
        Logger &logger = Logger::GetInstance();                     \
        logger.setLevel(LogLevel::ERROR);                           \
        char message[1024];                                         \
        snprintf(message, 1024, logMessageFormat, ##__VA_ARGS__);   \
        logger.log(message);                                        \
    } while (0);


#define LOG_FATAL(logMessageFormat, ...)                            \
    do {                                                            \
        Logger &logger = Logger::GetInstance();                     \
        logger.setLevel(LogLevel::FATAL);                           \
        char message[1024];                                         \
        snprintf(message, 1024, logMessageFormat, ##__VA_ARGS__);   \
        logger.log(message);                                        \
        exit(0);                                                    \
    } while (0);

/**
 * @brief 日志级别
 */
enum LogLevel {
    DEBUG,
    INFO,
    ERROR,
    FATAL,
};

/**
 * @brief 日志类
 */
class Logger : NonCopyable {
public:
    /// 获取 Logger 类的唯一实例
    static Logger &GetInstance();

    void setLevel(int level);

    void log(const std::string &message) const;

private:
    Logger() = default;

private:
    int m_level;
};

#endif //MUDUO_LOGGER_H
