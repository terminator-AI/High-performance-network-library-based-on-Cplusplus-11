#include <iostream>

#include "Logger.hpp"
#include "Timestamp.hpp"

//获取唯一实
Logger &Logger::instance()
{
    static Logger logger;
    return logger;
}
//设置日志水平
void Logger::setloggerLevel(int level)
{
    logLevel_ = level;
}
//打印日志
void Logger::log(std::string msg)
{
    switch (logLevel_)
    {
    case INFO:
        std::cout << "[INFO]--";
        break;
    case ERROR:
        std::cout << "[ERROR]--";
        break;
    case FATAL:
        std::cout << "[FATAL]--";
        break;
    case DEBUG:
        std::cout << "[DEBUG]--";
        break;
    default:
        break;
    }
    std::cout << TimesTamp::now().toString()
              << "  " << msg << std::endl;
}