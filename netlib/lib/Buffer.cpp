#include "Buffer.hpp"
#include <errno.h>
#include <unistd.h>
#include <sys/uio.h>
//从fd上读取数据到buffer中
ssize_t Buffer::readFd(int fd, int *saveErrno)
{
    char extrabuf[65536] = {0}; //栈上内存空间
    struct iovec vec[2];
    const size_t writable = wirtableBytes(); //buffer剩余大小
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;

    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);

    const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);
    if (n < 0)
    {
        *saveErrno = errno;
    }
    else if (n <= writable)
    {
        writerIndex_ += n;
    }
    else //extrabuf里面也写入了数据
    {
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable); //开始写n - writable的数据
    }
    return n;
}

//把buffer的数据通过fd发送
ssize_t Buffer::writeFd(int fd, int *saceErrno)
{
    ssize_t n = ::write(fd, peek(), readableBytes());
    if (n < 0)
    {
        *saceErrno = errno;
    }
    return n;
}