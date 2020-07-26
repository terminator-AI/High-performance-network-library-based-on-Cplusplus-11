#pragma once
#include <iostream>
#include <string>

//时间类
class TimesTamp
{
public:
    TimesTamp();
    explicit TimesTamp(int64_t microSecondsSinceEpoch);
    //获取当前时间
    static TimesTamp now();
    //格式转换
    std::string toString();

private:
    int64_t microSecondsSinceEpoch_;
};