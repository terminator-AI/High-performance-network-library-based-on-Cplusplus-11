#include "TcpServer.hpp"
#include "Logger.hpp"
#include "Channel.hpp"
#include "Acceptor.hpp"
#include <functional>
#include <strings.h>
static EventLoop *checkLoopNotNull(EventLoop *loop)
{
    if (loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d mainloop is nullptr\n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddress, std::string nameArg, Option option)
    : loop_(checkLoopNotNull(loop)), ipPort_(listenAddress.toIpPort()), name_(nameArg), acceptor_(new Acceptor(loop, listenAddress, option == kReusePort)), threadPool_(new EventLoopThreadPool(loop, name_)), connectionCallback_(), messageCallback_(), nextConnId_(1), started_(0)
{
    //当有新用户连接时，调用newConnection
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}
TcpServer::~TcpServer()
{
    for (auto &item : connectionMap_)
    {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestoryed, conn));
    }
}
//设置subloop个数
void TcpServer::setNumThread(int numThreads)
{
    threadPool_->setThreadNum(numThreads);
}

//开启服务器监听
void TcpServer::start()
{
    if (started_++ == 0) //防止服务多次启动
    {
        threadPool_->start(threadInitCallback_);                         //启动底层线程池
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get())); //开始在主loop中监听fd
    }
}
//有一个新的客户端的连接，会执行这个这个回调函数
void TcpServer::newConnection(int sockfd, const InetAddress &perrAdder)
{
    EventLoop *ioLoop = threadPool_->getNextLoop(); //轮询算法选择一个subloop来管理新的channel;
    char buf[64];
    snprintf(buf, 64, "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;
    LOG_INFO("TcpServer::newConnection [%s] - new conection [%s] from %s \n", name_.c_str(), connName.c_str(), perrAdder.toIpPort().c_str());

    //通过sockfd获取其绑定的本机ip地址和端口信息
    sockaddr_in local;
    bzero(&local, sizeof(local));
    socklen_t addrlen = sizeof(local);
    if (::getsockname(sockfd, (sockaddr *)&local, &addrlen) < 0)
    {
        LOG_ERROR("sockets::getLocalAddr");
    }
    InetAddress localAddr(local);

    //根据连接成功sockfd创建TcpConnection连接对象
    TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, localAddr, perrAdder));
    connectionMap_.insert({connName, conn});
    //用户设置给TcpServer--TcpConnection--Channel--Poller--notify channel调用
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    //直接调用
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}
void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
    LOG_INFO("TcpServer::removeConnectionInLoop() [%s] - connection %s \n", name_.c_str(), conn->name().c_str());
    connectionMap_.erase(conn->name());
    EventLoop *ioloop = conn->getLoop();
    ioloop->queueInLoop(std::bind(&TcpConnection::connectDestoryed, conn));
}