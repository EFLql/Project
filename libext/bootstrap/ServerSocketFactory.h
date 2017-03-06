#pragma once
#include <libext/asyn/AsyncSocketBase.h>
#include <libext/EventBaseManger.h>
#include <libext/SocketAddr.h>
#include <libext/asyn/AsyncServerSocket.h>
namespace libext
{
    
class ServerSocketFactory
{
public:
    ServerSocketFactory(){}
    virtual ~ServerSocketFactory() {}

    virtual std::shared_ptr<AsyncSocketBase> newSocket(
            libext::SocketAddr& addr, 
            int backlog, bool reuse) = 0;

    virtual void addAcceptCB(
            std::shared_ptr<AsyncSocketBase> s,
            Acceptor* callback, EventBase* env) = 0;
    virtual void removeAcceptCB(
            std::shared_ptr<AsyncSocketBase> s,
            Acceptor* callback, EventBase* env) = 0;
};

class AsyncServerSocketFactory : public ServerSocketFactory
{
public:
    AsyncServerSocketFactory() {}
    ~AsyncServerSocketFactory() {}
    virtual std::shared_ptr<AsyncSocketBase> newSocket(
            libext::SocketAddr& addr, 
            int backlog, bool reuse)
    {
        //EventBaseManager是一个管理EventBase对象的类
        //负责根据不同的线程返回线程私有的Eventbase对象
        EventBase* env = EventBaseManager::getInstanse()->getEventBase();
        //NOTE:facebook里面构造这个shared_ptr对象的时候还会传一个Destructor进去,这里先不传
        std::shared_ptr<AsyncServerSocket> socket(new AsyncServerSocket(env));
        socket->setReusePortEnable(reuse);
        socket->bind(addr);
        socket->listen(backlog);
        socket->startAccepting();

        return socket;
    }

    virtual void addAcceptCB(
            std::shared_ptr<AsyncSocketBase> s,
            Acceptor* callback, EventBase* env)
    {
        std::shared_ptr<AsyncServerSocket> socket = std::dynamic_pointer_cast<AsyncServerSocket>(s);
        socket->addAcceptCB(callback, env);
    }
    virtual void removeAcceptCB(
            std::shared_ptr<AsyncSocketBase> s,
            Acceptor* callback, EventBase* env)
    {
        std::shared_ptr<AsyncServerSocket> socket = std::dynamic_pointer_cast<AsyncServerSocket>(s);
        socket->removeAcceptCB(callback, env);
    }
};

class AsyncUDPServerSocketFactory
{
public:

private:
};

}//libext
