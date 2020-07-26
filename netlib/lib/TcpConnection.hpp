#pragma once

#include "noncopyable.hpp"
#include "InetAddress.hpp"
#include "CallBacks.hpp"
#include "Buffer.hpp"
#include "Timestamp.hpp"
#include <memory>
#include <string>
#include <atomic>
class Channel;
class EventLoop;
class Socket;
class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop *loop, const std::string nameArg, int sockfd, const InetAddress &localAddr, const InetAddress &peerAddr);
    ~TcpConnection();
    EventLoop *getLoop() const { return loop_; }
    const std::string &name() const { return name_; }
    const InetAddress &localAddress() const { return localAddr_; }
    const InetAddress &peerAddress() const { return peerAddr_; }
    bool connected() const { return state_ == kConnected; }

    //发送数据
    void send(const std::string &buf);
    //关闭连接
    void shutdown();

    void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }
    void setCloseCallback(const CloseCallback &cb) { closeCallback_ = cb; }
    void setHighWaterMarkCallback(const HighWaterMarkCallback &cb, size_t highWaterMark)
    {
        highWaterMarkCallback_ = cb;
        highWaterMark_ = highWaterMark;
    }

    //建立连接
    void connectEstablished();
    //销毁连接
    void connectDestoryed();

private:
    enum StateE
    {
        kDisconnected,
        kConnecting,
        kConnected,
        kDisconnecting
    };
    //设置连接状态
    void setState(StateE state) { state_ = state; }
    
    //事件发生回调函数
    void handleRead(TimesTamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void *message, size_t len);
    void shutdownInLoop();

    EventLoop *loop_; //subloop
    const std::string name_;
    std::atomic_int state_;
    bool reading_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    const InetAddress localAddr_;
    const InetAddress peerAddr_;

    ConnectionCallback connectionCallback_;       //有新连接时的回调
    MessageCallback messageCallback_;             //有消息读时的回调
    WriteCompleteCallback writeCompleteCallback_; //消息发送完以后的回调
    CloseCallback closeCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    size_t highWaterMark_;

    Buffer inputBuffer_;  //接受数据的缓存区
    Buffer outputBuffer_; //发送数据的缓存区
};