#include <libext/bootstrap/acceptor/acceptor.h>

void Acceptor::init(EventBase* evb)
{
        CHECK(base_ == NULL || evb == base_);
        base_ = evb;
}

void Acceptor::connectionAccepted(int fd,
                            const SocketAddr& clientAddr)
{
   //可以控制是否接受本次连接
   //if(canAccept(clientaddr)) ...

    //主要业务逻辑
    auto acceptTime = std::chrono::steady_clock::now();

    onDoneAcceptingConnection(fd, clientAddr, acceptTime); 
}

void Acceptor::onDoneAcceptingConnection(int fd, 
                                   const SocketAddr& clientAddr,
                                   std::chrono::steady_clock::time_point acceptTime)
{
    TransportInfo tinfo;
    processEstablishedConnection(fd, clientAddr, acceptTime, tinfo);
}

void Acceptor::processEstablishedConnection(int fd,
                                      const SocketAddr& clientAddr,
                                      std::chrono::steady_clock::time_point acceptTime,
                                      TransportInfo& tinfo)
{
    //lql-need add ssl code...
    bool shouldDoSSL = false;
    if(shouldDoSSL)
    {
        //do ssl
    }
    else
    {
        tinfo.secure = false;
        tinfo.acceptTime = acceptTime;
        //new AsyncSocket
    }
}

void Acceptor::acceptError(std::exception& e)
{
}

void Acceptor::acceptStarted()
{
}
