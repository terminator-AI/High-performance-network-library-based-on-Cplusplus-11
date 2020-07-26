#include "Poller.hpp"
#include "EpollPoller.hpp"
#include "stdlib.h"
class EventLoop;
Poller *Poller::newDefaultPoller(EventLoop *loop)
{
    if (::getenv("MUDUO_USE_POLL"))
    {
        return nullptr; //生成poll实例
    }
    else
    {
        return new EpollPoller(loop);
    }
}