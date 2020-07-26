#include <mymuduo/TcpServer.hpp>
#include <string>
#include <functional>
#include <mymuduo/Logger.hpp>

class EchServer
{
public:
    EchServer(EventLoop *loop, const InetAddress &addr, const std::string &name)
        : server_(loop, addr, name)
    {
        server_.setConnectionCallback(std::bind(&EchServer::onConnection, this, std::placeholders::_1));
        server_.setMessageCallback(std::bind(&EchServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        server_.setNumThread(3);
    }
    void start()
    {
        server_.start();
    }

private:
    void onConnection(const TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            LOG_INFO("CONN UP :%s", conn->peerAddress().toIpPort().c_str());
        }
        else
        {
            LOG_INFO("CONN DOWN :%s", conn->peerAddress().toIpPort().c_str());
        }
    }

    void onMessage(const TcpConnectionPtr &conn, Buffer *buffer, TimesTamp time)
    {
        std::string msg = buffer->retrieveAllString();
        conn->send(msg);
        conn->shutdown();
    }
    EventLoop *loop;
    TcpServer server_;
};

int main()
{
    EventLoop loop;
    InetAddress addr(8000, "127.0.0.1");
    EchServer server(&loop, addr, "EchServer_1");
    server.start();
    loop.loop();
    return 0;
}