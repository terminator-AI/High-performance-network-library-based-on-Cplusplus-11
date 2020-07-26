#pragma once

#include <strings.h>
#include <string>
#include <arpa/inet.h>
#include <netinet/in.h>

//封装地址类
class InetAddress
{
public:
    //地址绑定
    explicit InetAddress(int port = 0, std::string addr = "127.0.0.1");
    //地址绑定
    explicit InetAddress(const sockaddr_in &addr_in)
        : addr_(addr_in) {}
    //获取IP地址
    std::string toIp() const;
    //获取端口号
    uint16_t toPort() const;
    //获取IP+端口 ip:端口
    std::string toIpPort() const;
    //获取封装好的地址
    const sockaddr_in *getInetAddress() const { return &addr_; }
    void setSockAddr(const sockaddr_in &addr) { addr_ = addr; }

private:
    sockaddr_in addr_;
};