#include <libext/bootstrap/acceptor/Acceptor.h>

namespace libext
{
void Acceptor::init(EventBase* evb)
{
        CHECK(base_ == NULL || evb == base_);
        base_ = evb;
        state_ = State::kRunning;
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

void Acceptor::onDoneAcceptingConnection(
        int fd, 
        const SocketAddr& clientAddr,
        std::chrono::steady_clock::time_point acceptTime)
{
    TransportInfo tinfo;
    processEstablishedConnection(fd, clientAddr, acceptTime, tinfo);
}

void Acceptor::processEstablishedConnection(
        int fd,
        const SocketAddr& clientAddr,
        std::chrono::steady_clock::time_point acceptTime,
        TransportInfo& tinfo)
{
    //lql-need add ssl code...
    bool shouldDoSSL = false;
    if(shouldDoSSL)
    {
        //do ssl
        //sslConnectionReady(
        //        std::move(sock), 
        //        clientAddr,
        //        SecureTransportType::TLS,
        //        tinfo);

    }
    else
    {
        tinfo.secure = false;
        tinfo.acceptTime = acceptTime;
        //new AsyncSocket
        AsyncSocket::UniquePtr sock = std::make_unique<AsyncSocket>(base_, fd);
        plaintextConnectionReady(
                std::move(sock), 
                clientAddr,
                SecureTransportType::NONE,
                tinfo);
    }
}
void Acceptor::plaintextConnectionReady(
        AsyncSocket::UniquePtr sock, 
        const SocketAddr& clientAddr,
        SecureTransportType secureTransportType,
        TransportInfo& tinfo)
{
    connectionReady(std::move(sock),
                    clientAddr,
                    secureTransportType,
                    tinfo);

}

void Acceptor::sslConnectionReady(
        AsyncSocket::UniquePtr sock, 
        const SocketAddr& clientAddr,
        SecureTransportType secureTransportType,
        TransportInfo& tinfo)
{
    connectionReady(std::move(sock),
                    clientAddr,
                    secureTransportType,
                    tinfo);
}

void Acceptor::connectionReady(
        AsyncTransport::UniquePtr sock,
        const SocketAddr& clientAddr,
        SecureTransportType secureTransportType,
        TransportInfo& tinfo)
{
    //设置socket一次处理的数据量，同时保持内存的
    //使用率防止客户端往服务端疯狂的发数据，妨碍服务
    //端接受其他客户端的连接
    AsyncSocket* psock = dynamic_cast<AsyncSocket*>(sock.get());
    psock->setMaxReadsPerEvent(16);
    if(state_ < State::kDraining)//可以接受新的连接
    {
       onNewConnection(std::move(sock),
                        clientAddr,
                        secureTransportType,
                        tinfo);
    } 
}

void Acceptor::acceptError(const std::exception& e)
{
}

void Acceptor::acceptStarted()
{
}

void Acceptor::acceptStoped()
{

}

void Acceptor::addConnection(ManagedConnection* conn)
{
}

} //libext
