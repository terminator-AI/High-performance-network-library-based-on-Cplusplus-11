#pragma once
#include "noncopyable.hpp"
#include "Timestamp.hpp"
#include <vector>
#include <unordered_map>
class Channel;
class EventLoop;

//多路事件分发器的核心IO复用模块
class Poller : noncopyable
{
public:
    using ChannelList = std::vector<Channel *>;
    
    Poller(EventLoop *loop) : ownerLoop_(loop) {}
    virtual ~Poller() = default;

    //给所有IO复用保留统一接口
    virtual TimesTamp poll(int timeouts, ChannelList *activeChannels) = 0;
    virtual void updateChannel(Channel *channel) = 0;
    virtual void removeChannel(Channel *channel) = 0;
    //判断channel是否在当前poller当中
    bool hasChannel(Channel *channel) const;
    //EventLoop可以通过该接口获取默认的IO复用的具体实现
    static Poller *newDefaultPoller(EventLoop *loop);

protected:
    using ChannelMap = std::unordered_map<int, Channel *>;
    ChannelMap channels_;

private:
    EventLoop *ownerLoop_; //定义poller所属事件循环EventLoop
};