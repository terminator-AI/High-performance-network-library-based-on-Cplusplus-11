#pragma once
#include "noncopyable.hpp"
#include "Socket.hpp"
#include "Channel.hpp"
#include <functional>
class EventLoop;
class InetAddress;
class Acceptor
{
public:
    using NewConnectionCallBack = std::function<void(int sokefd, const InetAddress &)>;
    Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallBack &cb)
    {
        newConnectionCallback_ = cb;
    }
    bool listenning() { return listenning_; }
    void listen();

private:
    void handleRead();
    EventLoop *loop_; //使用的baseloop
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallBack newConnectionCallback_;
    bool listenning_;
};