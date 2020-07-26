#include "Channel.hpp"
#include "EventLoop.hpp"
#include "Logger.hpp"
#include <sys/epoll.h>
const int Channel::kNoneEvent = 0;                  //无事件
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI; //读事件
const int Channel::kWriteEvent = EPOLLOUT;          //写事件
Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1), tied_(false)
{
}

Channel::~Channel()
{
}

void Channel::tie(const std::shared_ptr<void> &obj)
{
    tie_ = obj;
    tied_ = true;
}
//向poller更新fd修改后的events事件
void Channel::update()
{
    //通过channel所属的eventloop，调用相应的方法注册fd的events事件
    loop_->updateChannel(this);
}
//在channel所属的eventloop删除channel
void Channel::remove()
{
    loop_->removeChannel(this);
}

//fd得到poller通知，处理事件
void Channel::handleEvent(TimesTamp receiveTime)
{
    if (tied_)
    {
        std::shared_ptr<void> gruad = tie_.lock();
        if (gruad)
        {
            handleEventWithGuard(receiveTime);
        }
    }
    else
    {
        handleEventWithGuard(receiveTime);
    }
}
//根据poller通知的具体事件，调用相应的回调操作
void Channel::handleEventWithGuard(TimesTamp receiveTime)
{
    LOG_INFO("Channel EventHandle Events:%d\n", revents_);
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
    {
        if (closeCallBack_)
        {
            closeCallBack_();
        }
    }
    if (revents_ & EPOLLERR)
    {
        if (errorCallBack_)
        {
            errorCallBack_();
        }
    }
    if (revents_ & EPOLLIN)
    {
        if (readCallBack_)
        {
            readCallBack_(receiveTime);
        }
    }
    if (events_ & EPOLLOUT)
    {
        if (writeCallBack_)
        {
            writeCallBack_();
        }
    }
}
