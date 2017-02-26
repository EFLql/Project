#pragma once
#include <libext/asyn/AsyncSocketBase.h>
#include <libext/EventBaseManger.h>

namespace libext
{
    
class ServerSocketFactory
{
public:
    ServerSocketFactory();
    virtual ~ServerSocketFactory();

    virtual std::shared_ptr<AsyncSocketBase> newSocket(
            libext::SocketAddr& addr, 
            int backlog, bool reuse) = 0;

    virtual addAcceptCB(
            std::shared_ptr<AsyncSocketBase> socket,
            Acceptor* callback, EventBase* env) = 0;
    virtual removeAcceptCB(
            std::shared_ptr<AsyncSocketBase> socket,
            Acceptor* callback, EventBase* env) = 0;
};

class AsyncServerSocketFactory : public ServerSocketFactory
{
public:
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

    virtual addAcceptCB(
            std::shared_ptr<AsyncSocketBase> socket,
            Acceptor* callback, EventBase env)
    {
        socket->addAcceptCB(callback, env);
    }
    virtual removeAcceptCB(
            std::shared_ptr<AsyncSocketBase> socket,
            Acceptor* callback, EventBase* env)
    {
        socket->removeAcceptCB(callback, env);
    }
};

class AsyncUDPServerSocketFactory
{
public:

private:
};

}//libext
