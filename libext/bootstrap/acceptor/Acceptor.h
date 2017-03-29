#pragma once
#include <libext/asyn/AsyncServerSocket.h>
#include <libext/asyn/EventBase.h>
#include <libext/EventBaseManger.h>
#include <libext/bootstrap/acceptor/TransportInfo.h>

//Acceptor类控制客户端连接（接受连接还是丢弃),根据底层socket
//创建具体功能的上层socket对象如SSL或者普通的socket对象
//并设置底层socket属性
namespace libext
{
class Acceptor : public AsyncServerSocket::AcceptCallback
{
public:
    Acceptor() {}
    ~Acceptor() {}
    void init(EventBase* evb);

    EventBase* getEventBase() const
    {
        return base_;
    }
    
    void connectionAccepted(int fd,
                            const SocketAddr& clientAddr) override;
    void acceptError(std::exception& e) override;
    void acceptStarted() override;
    void onDoneAcceptingConnection(int fd, 
                                   const SocketAddr& clientAddr,
                                   std::chrono::steady_clock::time_point acceptTime);
    void processEstablishedConnection(int fd,
                                      const SocketAddr& clientAddr,
                                      std::chrono::steady_clock::time_point acceptTime,
                                      TransportInfo& tinfo);

private:
    EventBase* base_;
};


class AcceptorFactory
{
public:
    AcceptorFactory() {}
    ~AcceptorFactory() {}

    std::shared_ptr<Acceptor> newAcceptor(EventBase* evb)
    {
        std::shared_ptr<Acceptor> acceptor = std::make_shared<Acceptor>();
        acceptor->init(evb);
        return acceptor;
    } 
};

} //libext
