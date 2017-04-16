#pragma once
#include <libext/asyn/AsyncServerSocket.h>
#include <libext/asyn/EventBase.h>
#include <libext/EventBaseManger.h>
#include <libext/bootstrap/acceptor/TransportInfo.h>
#include <libext/asyn/AsyncSocket.h>
#include <libext/bootstrap/acceptor/ManagedConnection.h>

//Acceptor类控制客户端连接（接受连接还是丢弃),根据底层socket
//创建具体功能的上层socket对象如SSL或者普通的socket对象
//并设置底层socket属性
namespace libext
{
//enum class c++中对enum的缺陷修复,
//原始的enum可以被隐式转换为int型的变量造成
//类型安全方面的问题,还有其他夸平台问题
//www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2347.pdf
enum class SecureTransportType
{
    NONE,//Transport is no secure
    TLS,//Transport is base on TLS(transport layer secure)
    ZERO,//Transport is base on zero
};

class Acceptor : public AsyncServerSocket::AcceptCallback
{
public:
    enum class State : uint32_t
    {
        kInit,//还没开始
        kRunning, //正常处理请求
        kDraining, //处理正常连接，但不accepting 新的连接
        kDone, //不再accepting,并且所有的连接已经完成
    };
    Acceptor() {}
    ~Acceptor() {}
    virtual void init(EventBase* evb);

    EventBase* getEventBase() const
    {
        return base_;
    }
    
    void connectionAccepted(int fd,
                            const SocketAddr& clientAddr) override;
    void acceptError(const std::exception& e) override;
    void acceptStarted() override;
    void acceptStoped() override;
    void onDoneAcceptingConnection(int fd, 
                                   const SocketAddr& clientAddr,
                                   std::chrono::steady_clock::time_point acceptTime);
    void processEstablishedConnection(int fd,
                                      const SocketAddr& clientAddr,
                                      std::chrono::steady_clock::time_point acceptTime,
                                      TransportInfo& tinfo);
    void plaintextConnectionReady(AsyncSocket::UniquePtr sock, 
                                  const SocketAddr& clientAddr,
                                  SecureTransportType secureTransportType,
                                  TransportInfo& tinfo);
    void sslConnectionReady(AsyncSocket::UniquePtr sock, 
                                  const SocketAddr& clientAddr,
                                  SecureTransportType secureTransportType,
                                  TransportInfo& tinfo);
    void connectionReady(AsyncTransport::UniquePtr sock,
                                  const SocketAddr& clientAddr,
                                  SecureTransportType secureTransportType,
                                  TransportInfo& tinfo);
                        
    virtual void onNewConnection(AsyncTransport::UniquePtr sock,
                                  const SocketAddr& clientAddr,
                                  SecureTransportType secureTransportType,
                                  TransportInfo& tinfo){}
    void addConnection(ManagedConnection* conn);
private:
    EventBase* base_;
    State state_{State::kInit};
};


class AcceptorFactory
{
public:
    AcceptorFactory() {}
    ~AcceptorFactory() {}

    virtual std::shared_ptr<Acceptor> newAcceptor(EventBase* evb)
    {
        std::shared_ptr<Acceptor> acceptor = std::make_shared<Acceptor>();
        acceptor->init(evb);
        return acceptor;
    } 
};

} //libext
