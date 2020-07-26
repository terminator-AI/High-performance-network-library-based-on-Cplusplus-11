#include "Poller.hpp"
#include "Channel.hpp"
//判断channel是否在当前poller当中
bool Poller::hasChannel(Channel *channel) const
{
    auto it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel;
}
