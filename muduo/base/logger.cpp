//
// Created by liucxi on 2022/10/12.
//

#include "logger.h"
#include "times.h"

#include <iostream>

Logger &Logger::GetInstance() {
    static Logger logger;
    return logger;
}

void Logger::setLevel(int level) {
    m_level = level;
}

void Logger::log(const std::string &message) const {
    static std::string g_LogLeveL[] = {"DEBUG", "INFO", "ERROR", "FATAL"};
    std::cout << g_LogLeveL[m_level] << Time::now().toString() << " : " << message << std::endl;
}