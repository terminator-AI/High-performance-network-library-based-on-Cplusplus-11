#pragma once
#include <string>

#include "noncopyable.hpp"

//LOG_XX(% % %,...)
#define LOG_INFO(logMsgFormat, ...)                       \
    do                                                    \
    {                                                     \
        Logger &logger = Logger::instance();              \
        logger.setloggerLevel(INFO);                      \
        char buf[1024] = {0};                             \
        snprintf(buf, 1024, logMsgFormat, ##__VA_ARGS__); \
        logger.log(buf);                                  \
    } while (0)

#define LOG_ERROR(logMsgFormat, ...)                      \
    do                                                    \
    {                                                     \
        Logger &logger = Logger::instance();              \
        logger.setloggerLevel(ERROR);                     \
        char buf[1024] = {0};                             \
        snprintf(buf, 1024, logMsgFormat, ##__VA_ARGS__); \
        logger.log(buf);                                  \
    } while (0)

#define LOG_FATAL(logMsgFormat, ...)                      \
    do                                                    \
    {                                                     \
        Logger &logger = Logger::instance();              \
        logger.setloggerLevel(FATAL);                     \
        char buf[1024] = {0};                             \
        snprintf(buf, 1024, logMsgFormat, ##__VA_ARGS__); \
        logger.log(buf);                                  \
        exit(-1);                                         \
    } while (0)

#ifdef MUDEBUG
#define LOG_DEBUG(logMsgFormat, ...)                      \
    do                                                    \
    {                                                     \
        Logger &logger = Logger::instance();              \
        logger.setloggerLevel(DEBUG);                     \
        char buf[1024] = {0};                             \
        snprintf(buf, 1024, logMsgFormat, ##__VA_ARGS__); \
        logger.log(buf);                                  \
    } while (0)
#else
#define LOG_DEBUG(logMsgFormat, ...)
#endif
//"日志系统"
enum
{
    INFO,  //普通信息
    ERROR, //错误信息
    FATAL, //CORE信息
    DEBUG, //调试信息
};

//日志类
class Logger
{
public:
    //获取唯一实例
    static Logger &instance();
    //设置日志水平
    void setloggerLevel(int level);
    //打印日志
    void log(std::string msg);

private:
    int logLevel_;
    Logger() {}
};
