#include "Timestamp.hpp"
#include <ctime>

TimesTamp::TimesTamp() : microSecondsSinceEpoch_(0) {}
TimesTamp::TimesTamp(int64_t microSecondsSinceEpoch)
    : microSecondsSinceEpoch_(microSecondsSinceEpoch) {}
//获取当前时间
TimesTamp TimesTamp::now()
{
    return TimesTamp(time(NULL));
}
//格式转换
std::string TimesTamp::toString()
{
    char buf[128] = {0};
    tm *tm_time = localtime(&microSecondsSinceEpoch_);
    sprintf(buf, "%04d/%02d/%02d %02d:%02d:%02d",
            tm_time->tm_year + 1900,
            tm_time->tm_mon + 1,
            tm_time->tm_mday,
            tm_time->tm_hour,
            tm_time->tm_min,
            tm_time->tm_sec);
    return buf;
}

// #include <iostream>

// int main()
// {
//     std::cout << TimesTamp::now().toString();
//     return 0;
// }