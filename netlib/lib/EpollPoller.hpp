#include "Poller.hpp"
#include <vector>
#include <sys/epoll.h>
class EpollPoller : public Poller
{
public:
    EpollPoller(EventLoop *loop);
    ~EpollPoller() override;
    //重写基类poller的抽象方法
    TimesTamp poll(int timeouts, ChannelList *activeChannels) override;
    void updateChannel(Channel *channel) override;
    void removeChannel(Channel *channel) override;

private:
    static const int kInitEventListSize = 16;
    using EventList = std::vector<epoll_event>;

    //填写活跃的连接
    void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;
    //更新channel通道
    void update(int operation, Channel *channel);

    int epollfd_;
    EventList events_;
};
