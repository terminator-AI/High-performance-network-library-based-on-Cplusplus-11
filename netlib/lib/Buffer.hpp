#pragma once
#include <vector>
#include <string>
#include <algorithm>
class Buffer
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;
    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize), readerIndex_(kCheapPrepend), writerIndex_(kCheapPrepend)
    {
    }

    size_t readableBytes() const { return writerIndex_ - readerIndex_; }
    size_t wirtableBytes() const { return buffer_.size() - writerIndex_; }
    size_t prependableBytes() const { return readerIndex_; }

    //把onMessage函数上报的buffer数据转成string类型
    std::string retrieveAllString()
    {
        return retrivevAsString(readableBytes());
    }
    std::string retrivevAsString(size_t len)
    {
        std::string result(peek(), len);
        retrieve(len); //对缓冲区复位操作
        return result;
    }
    //返回缓冲区中可读数据去的起始地址
    const char *peek() const
    {
        return begin() + readerIndex_;
    }
    //buffer--->string
    void retrieve(size_t len)
    {
        if (len < readableBytes())
        {
            readerIndex_ += len;
        }
        else
        {
            retrieveAll();
        }
    }
    void retrieveAll()
    {
        readerIndex_ = writerIndex_ = kCheapPrepend;
    }

    void ensureWritableBytes(size_t len)
    {
        if (wirtableBytes() < len)
        {
            makeSpace(len); //扩容
        }
    }
    //把data内存上的数据添加到buffer中
    void append(const char *data, size_t len)
    {
        ensureWritableBytes(len);
        std::copy(data, data + len, beginWrite());
        writerIndex_ += len;
    }
    char *beginWrite()
    {
        return begin() + writerIndex_;
    }
    const char *beginWrite() const
    {
        return begin() + writerIndex_;
    }
    //从fd上读取数据到buffer中
    ssize_t readFd(int fd, int *saveErrno);
    //把buffer的数据通过fd发送
    ssize_t writeFd(int fd, int *saceErrno);

private:
    char *begin()
    {
        return &*buffer_.begin(); //buffer内部数组的首地址
    }
    const char *begin() const
    {
        return &*buffer_.begin();
    }
    void makeSpace(size_t len)
    {
        if (wirtableBytes() + prependableBytes() < len + kCheapPrepend)
        {
            buffer_.resize(writerIndex_ + len);
        }
        else
        {
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_, begin() + writerIndex_, begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};