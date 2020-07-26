#include "EpollPoller.hpp"
#include "Logger.hpp"
#include "Channel.hpp"
#include <unistd.h>
#include <memory.h>
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;

EpollPoller::EpollPoller(EventLoop *loop)
    : Poller(loop), epollfd_(epoll_create1(EPOLL_CLOEXEC)), events_(kInitEventListSize)
{
    if (epollfd_ < 0)
    { //epoll失败
        LOG_FATAL("epoll_create error:%d", errno);
    }
}
EpollPoller::~EpollPoller()
{
    ::close(epollfd_);
}
//重写基类poller的抽象方法
TimesTamp EpollPoller::poll(int timeouts, ChannelList *activeChannels)
{
    //实则用LOG_DEBUG更好，不会大量显示日志信息
    LOG_INFO("func=%s ==> fd total count:%lu \n", __FUNCTION__, channels_.size());
    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeouts);
    int saveErrno = errno;
    TimesTamp now(TimesTamp::now());
    if (numEvents > 0)
    {
        LOG_INFO("%d events happened \n", numEvents);
        fillActiveChannels(numEvents, activeChannels);
        if (numEvents == events_.size())
        {
            events_.resize(events_.size() * 2);
        }
    }
    else if (numEvents == 0)
    {
        LOG_DEBUG("%s timeout! \n", __FUNCTION__);
    }
    else
    {
        if (saveErrno != EINTR)
        {
            errno = saveErrno;
            LOG_ERROR("EpollPoller::poll() err!");
        }
    }
    return now;
}
//更新poller中的channel
void EpollPoller::updateChannel(Channel *channel)
{
    const int index = channel->index();
    LOG_INFO("func:%s ==> fd=%d,events=%d,index=%d \n", __FUNCTION__, channel->fd(), channel->events(), index);
    if (index == kNew || index == kDeleted)
    {
        if (index == kNew)
        { //如果没有在poller中，添加到channels_中
            int fd = channel->fd();
            channels_[fd] = channel;
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else
    {
        int fd = channel->fd();
        if (channel->isNoneEvent())
        { //如果添加了，但是没有感兴趣的事件
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}
//移除Poller中的channel
void EpollPoller::removeChannel(Channel *channel)
{

    int fd = channel->fd();
    LOG_INFO("func:%s ==> fd=%d \n", __FUNCTION__, channel->fd());
    channels_.erase(fd);
    int index = channel->index();
    if (index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}
//填写活跃的连接
void EpollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const
{
    for (int i = 0; i < numEvents; ++i)
    {
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr); //void*-->Chanel*
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}
//更新channel通道,更新epoll中的channel
void EpollPoller::update(int operation, Channel *channel)
{
    epoll_event event;
    memset(&event, 0, sizeof(event));

    int fd = channel->fd();
    event.data.fd = channel->fd();
    event.events = channel->events();
    event.data.ptr = channel;
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        if (operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("Epoll del error:%d \n", EPOLL_CTL_DEL);
        }
        else
        {
            LOG_FATAL("Epoll add/mod error:%d \n", operation);
        }
    }
}