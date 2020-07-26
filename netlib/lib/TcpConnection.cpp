#include "TcpConnection.hpp"
#include "Logger.hpp"
#include "Socket.hpp"
#include "Channel.hpp"
#include "EventLoop.hpp"
#include "Buffer.hpp"
#include <errno.h>
#include <functional>
static EventLoop *checkLoopNotNull(EventLoop *loop)
{
    if (loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d TcpConnection is nullptr\n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}
TcpConnection::TcpConnection(EventLoop *loop, const std::string nameArg, int sockfd, const InetAddress &localAddr, const InetAddress &peerAddr)
    : loop_(checkLoopNotNull(loop)), name_(nameArg), state_(kConnecting), reading_(true), socket_(new Socket(sockfd)), channel_(new Channel(loop_, sockfd)), localAddr_(localAddr), peerAddr_(peerAddr), highWaterMark_(64 * 1024 * 1024) //64M
{
    //给channel设置相应的回调函数，poller给channel通知感兴趣的事件会调用
    channel_->setReadCallBack(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_->setWriteCallBack(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallBack(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallBack(std::bind(&TcpConnection::handleError, this));

    LOG_INFO("TcpConnection::stor[%s] at fd=%d\n", name_.c_str(), sockfd);
    socket_->setKeepAlive(true);
}
TcpConnection::~TcpConnection()
{
    LOG_INFO("TcpConnection::dtor[%s] at fd=%d state=%d\n", name_.c_str(), channel_->fd(), static_cast<int>(state_));
}

//关闭连接
void TcpConnection::shutdown()
{
    if (state_ == kConnected)
    {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}
void TcpConnection::shutdownInLoop()
{
    if (!channel_->iskWriteEvent()) //说明当前outputbuffer中的数据已经发送完成
    {
        socket_->shutdownWrite(); //关闭写端  触发EPOLLHUP事件--->从而执行handleClose
    }
}
//建立连接
void TcpConnection::connectEstablished()
{
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading(); //向poller注册读事件

    //新连接建立，建立回调
    connectionCallback_(shared_from_this());
}
//销毁连接
void TcpConnection::connectDestoryed()
{
    if (state_ == kConnected)
    {
        setState(kDisconnected);
        channel_->disableAll(); //把channel所有感兴趣事件从poller中去掉
        connectionCallback_(shared_from_this());
    }
    channel_->remove(); //把channel从poller中删除
}
void TcpConnection::handleRead(TimesTamp receiveTime)
{
    int saveErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &saveErrno);
    if (n > 0)
    {
        //已建立的用户有可读事件发生了，调用用户传入的回调操作
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }
    else if (n == 0)
    {
        handleClose();
    }
    else
    {
        errno = saveErrno;
        LOG_ERROR("TcpConnection::handleRead\n");
        handleError();
    }
}
void TcpConnection::handleWrite()
{
    if (channel_->iskWriteEvent())
    {
        int saveErrno = 0;
        ssize_t n = outputBuffer_.writeFd(channel_->fd(), &saveErrno);
        if (n > 0)
        {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0)
            {
                channel_->disableWriting();
                if (writeCompleteCallback_)
                {
                    //唤醒loop_对应的thread线程，执行回调
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if (state_ == kDisconnecting)
                {
                    shutdownInLoop();
                }
            }
        }
        else
        {
            LOG_ERROR("TcpConnection::handleWrite\n");
        }
    }
    else
    {
        LOG_ERROR("TcpConnection fd=%d is down,no more writing\n", channel_->fd());
    }
}
void TcpConnection::handleClose()
{
    LOG_INFO("fd=%d state=%d\n", channel_->fd(), static_cast<int>(state_));
    setState(kDisconnected);
    channel_->disableAll();
    TcpConnectionPtr connPtr(shared_from_this());
    connectionCallback_(connPtr); //执行连接关闭的回调
    closeCallback_(connPtr);      //关闭连接的回调 <--TCPserverTcpServer::removeConnection
}
void TcpConnection::handleError()
{
    int optval;
    socklen_t optlen = sizeof(optval);
    int err = 0;
    if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        err = errno;
    }
    else
    {
        err = optval;
    }
    LOG_ERROR("TcpConnection::handleError() name:%s - SO_ERROR:%d\n", name_.c_str(), err);
}

void TcpConnection::send(const std::string &buf)
{
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(buf.c_str(), buf.size());
        }
        else
        {
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, buf.c_str(), buf.size()));
        }
    }
}

//发送数据，应用写的快，内核发送数据慢，需要把数据写入buffer中，设置水位回调，防止发送过快
void TcpConnection::sendInLoop(const void *data, size_t len)
{
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;
    if (state_ == kDisconnected) //之前调用过connection的shutdown
    {
        LOG_ERROR("Disconnected,give up writing\n");
        return;
    }
    //表示channel第一次开始写数据，而且缓冲区没有待发送数据
    if (!channel_->iskWriteEvent() && outputBuffer_.readableBytes() == 0)
    {
        nwrote = ::write(channel_->fd(), data, len);
        if (nwrote >= 0)
        {
            remaining = len - nwrote;
            if (remaining == 0 && writeCompleteCallback_)
            {
                //一次性发送完成，就不用再给channel设置epollout事件
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        }
        else
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK)
            {
                LOG_ERROR("TcpConnection::senInLoop");
                if (errno == EPIPE || errno == ECONNRESET) //sigpipe reset;
                {
                    faultError = true;
                }
            }
        }
    }
    //说明当前这次write并没有把数据全部发送出去，需要将剩余数据保存到缓冲区中，并给channe注册epollout事件
    //poller发现tcp的发送缓冲区有空间，会通知相应的socket-channel，调用writeCallBack回调
    //最终调用handlewrite方法，把发送缓冲区中的数据全部发送完成
    if (!faultError && remaining > 0)
    {
        //目前缓冲区剩发送的数据
        size_t oldLen = outputBuffer_.readableBytes();
        if (oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_ && highWaterMarkCallback_)
        {
            loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
        }
        outputBuffer_.append(static_cast<const char *>(data) + nwrote, remaining);
        if (!channel_->iskWriteEvent())
        {
            channel_->enableWriting(); //一定要设置，否则poller不会通知epollout事件的发生
        }
    }
}
