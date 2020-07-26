#pragma once


//nocopyable被继承后，派生类可构造和析构，但不能拷贝
class noncopyable
{
public:
    noncopyable(const noncopyable &) = delete;
    void operator=(const noncopyable &) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;
};
