#pragma once
#include "noncopyable.hpp"
#include "EventLoop.hpp"
#include "EventLoopThreadPool.hpp"
#include "TcpConnection.hpp"
#include "Acceptor.hpp"
#include "InetAddress.hpp"
#include "CallBacks.hpp"
#include "Buffer.hpp"
#include <functional>
#include <string>
#include <memory>
#include <atomic>
#include <unordered_map>
class TcpServer : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;
    enum Option
    {
        kNoReusePort,
        kReusePort
    };

    TcpServer(EventLoop *loop, const InetAddress &listenAddress, std::string nameArg, Option option = kNoReusePort);
    ~TcpServer();

    void setThreadIninCallback(const ThreadInitCallback &cb) { threadInitCallback_ = cb; }
    void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }

    //设置subloop个数
    void setNumThread(int numThreads);

    //开启服务器监听
    void start();

private:
    void newConnection(int sockfd, const InetAddress &perrAdder);
    void removeConnection(const TcpConnectionPtr &conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);
    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

    EventLoop *loop_; //baseloop
    const std::string name_;
    const std::string ipPort_;

    std::unique_ptr<Acceptor> acceptor_;              //运行在mainloop，任务：监听新用户连接
    std::shared_ptr<EventLoopThreadPool> threadPool_; //loop线程池

    ConnectionCallback connectionCallback_;       //有新连接时的回调
    MessageCallback messageCallback_;             //有消息读时的回调
    WriteCompleteCallback writeCompleteCallback_; //消息发送完以后的回调
    ThreadInitCallback threadInitCallback_;       //loop线程初始化回调

    std::atomic_int started_;

    int nextConnId_;

    ConnectionMap connectionMap_; //保存所有的连接
};