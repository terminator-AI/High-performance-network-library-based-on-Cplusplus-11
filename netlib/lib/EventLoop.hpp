#pragma once
#include "noncopyable.hpp"
#include "Timestamp.hpp"
#include "CurrentThread.hpp"
#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>
class Channel;
class Poller;
//事件循环类，包含channel poller(epoll抽象)
class EventLoop : noncopyable
{
public:
    using Functor = std::function<void()>;
    EventLoop();
    ~EventLoop();

    void loop(); //开启事件循环
    void quit(); //退出事件循环
    TimesTamp pollReturnTime() const { return poolReturnTime_; }

    void runInLoop(Functor cb);   //在当前loop中执行cb
    void queueInLoop(Functor cb); //把cb放入队列，唤醒loop所在的线程，执行cb

    void wakeup(); //唤醒loop所在线程

    void updateChannel(Channel *channel); //EventLoop方法==>调用Poller的方法
    void removeChannel(Channel *channel);
    void hasChannel(Channel *channel);

    //判断loop对象是否在自己的线程里
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

private:
    void handleRead();        //wakeup
    void doPendingFunctors(); //执行回调

    using ChannelList = std::vector<Channel *>;
    std::atomic_bool looping_; /* 原子操作 */
    std::atomic_bool quit_;    /* 标志退出loop循环 */

    const pid_t threadId_;     //记录当前线程loop的ID
    TimesTamp poolReturnTime_; //记录当前poller返回发生事件的channels的时间点
    std::unique_ptr<Poller> poller_;
    int wakeupfd_; //当mainloop获取一个新用户的，用过轮询算法选择一个subloop，通过该成员唤醒subchannel处理channel
    std::unique_ptr<Channel> wakeupChannel_;
    ChannelList activeChannels_;

    std::atomic_bool callingPendingFunctors_; //标识当前loop是否有需要执行的回调操作
    std::vector<Functor> pendingFunctors_;    //存储loop需要执行的所有回调操作
    std::mutex mutex_;                        //互斥锁，用来保护pendingFunctors容器的线程安全
};