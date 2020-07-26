#pragma once
#include <functional>
#include <memory>

#include "noncopyable.hpp"
#include "Timestamp.hpp"

class EventLoop;

//channel理解为通道，包含socketfd，Event，reEvent
class Channel : noncopyable
{
public:
    using EventCallBack = std::function<void()>;
    using ReadCallBack = std::function<void(TimesTamp)>;
    Channel(EventLoop *loop, int fd);
    ~Channel();

    //fd得到poller通知，处理事件
    void handleEvent(TimesTamp receiveTime);

    //设置回调函数对象
    void setReadCallBack(ReadCallBack cb) { readCallBack_ = std::move(cb); }
    void setWriteCallBack(EventCallBack cb) { writeCallBack_ = std::move(cb); }
    void setCloseCallBack(EventCallBack cb) { closeCallBack_ = std::move(cb); }
    void setErrorCallBack(EventCallBack cb) { errorCallBack_ = std::move(cb); }

    //阻止当channel被手动remove掉时channel还在执行回调操作
    void tie(const std::shared_ptr<void> &);

    int fd() const { return fd_; }
    int events() const { return events_; }

    //提供外部接口，当poller通知有事件发生时，设置实际发生的事件
    int set_revents(int revt) { revents_ = revt; }

    //设置fd相应的事件状态
    void enableReading()
    {
        events_ |= kReadEvent;
        update();
    }
    void disableReading()
    {
        events_ &= ~kReadEvent;
        update();
    }
    void enableWriting()
    {
        events_ |= kWriteEvent;
        update();
    }
    void disableWriting()
    {
        events_ &= ~kWriteEvent;
        update();
    }
    void disableAll()
    {
        events_ = kNoneEvent;
        update();
    }

    //判断fd有没有设置感兴趣的事件
    bool isNoneEvent() const { return events_ == kNoneEvent; }
    bool iskReadEvent() const { return events_ == kReadEvent; }
    bool iskWriteEvent() const { return events_ == kWriteEvent; }

    int index() { return index_; }
    void set_index(int idx) { index_ = idx; }

    EventLoop *ownerLoop() { return loop_; }
    void remove();

private:
    void update();
    void handleEventWithGuard(TimesTamp receiveTime);

    static const int kNoneEvent;  //无事件
    static const int kReadEvent;  //读事件
    static const int kWriteEvent; //写事件

    EventLoop *loop_; //事件循环
    const int fd_;    //fd poller监听的描述符
    int events_;      //注册fd感兴趣的事件
    int revents_;     //poller返回的具体发送事件
    int index_;       //判断channel是否注册在epoll中

    std::weak_ptr<void> tie_;
    bool tied_;

    //因为channel可以获知fd最终发生的事件，所以channel负责调用具体事件的回调操作
    ReadCallBack readCallBack_;
    EventCallBack writeCallBack_;
    EventCallBack closeCallBack_;
    EventCallBack errorCallBack_;
};