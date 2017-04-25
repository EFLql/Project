#include <libext/asyn/AsyncSocket.h>
#include <assert.h>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

namespace libext
{
//server端
AsyncSocket::AsyncSocket(EventBase* evb, int fd)
: maxReadsPerEvent_(16)
, readCallback_(NULL)
, fd_(fd)
, eventBase_(evb)
, ioHandler_(evb, fd, this)
{
    //服务端使用，在AsyncSocket构造之前
    //和客户端的连接就已经建立了，所以
    //这里初始时状态就是连接建立成功了的
    state_ = StateEnum::ESTABLISHED;
}

//客户端
AsyncSocket::AsyncSocket()
: maxReadsPerEvent_(16)
, readCallback_(NULL)
, state_(StateEnum::UNINT)
, ioHandler_(NULL, 0, this)
{

}
AsyncSocket(EventBase* evb, const SocketAddr& addr,
            uint32_t connectTimeout)
    :AsyncSocket(evb)

{
    connect(NULL, addr, connectTimeout);
}

AsyncSocket(EventBase* evb, const std::string& ip,
            uint16_t port, uint32_t connectTimeout)
    :AsyncSocket(evb)
{
    connect(NULL, SocketAddr(ip, port), connectTimeout);
}


AsyncSocket::~AsyncSocket()
{
}

void connect(ConnectCallback* callback, const SocketAddr& addr,
            uint32_t connectTimeout) noexcept
{
    assert(eventBase_->isRunningEventBase());
    if(state_ != StateEnum::UNINT)
    {
        return invalidState(callback);
    }

    connectCallback_ = callback;
    //
    assert(fd_ == -1);

    try
    {
        fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if(fd_ < 0)
        {
            throw AsyncSocketException(
                    AsyncSocketException::INTERNAL_ERROR,
                    "Create socket failed",
                     errno);
        }
        //设置socket属性为非阻塞模式
        int flags = fcntl(fd_, F_GETFD, 0);
        if(flags < 0)
        {
            throw AsyncSocketException(
                    AsyncSocketException::INTERNAL_ERROR,
                    "Get flag failed on socket",
                     errno);
        }
        if(fcntl(fd_, F_SETFL, flags | O_NONBLOCK) < 0)
        {
            throw AsyncSocketException(
                    AsyncSocketException::INTERNAL_ERROR,
                    "Set NONBLOCK failed on socket",
                     errno);
        }
        //connect
        socketConnect(addr);
        
    }catch (const AsyncSocketException& ex)
    {
    }
}

void connect(ConnectCallback* callback, const std::string& ip,
            uint16_t port, uint32_t connectTimeout) noexcept
{
    try
    {
        connect(callback, SocketAddr(ip, port), connectTimeout);
    }catch(const std::exception& ex)
    {
        AsyncSocketException tex(
                AsyncSocketException::INTERNAL_ERROR,
                ex.what());
        failConnect(tex);
    }
}

void AsyncSocket::socketConnect(const SocketAddr& addr)
{
    assert(fd_ != -1);
    int ret = ::connect(fd_, 
                        (struct sockaddr*)&addr.getSocketAddr(),
                        sizeof(addr.getSocketAddr()));
    if(ret < 0)
    {
        if(errno == EINPROGRESS)//异步模式下连接正在进行
        {
        }
        else
        {
            throw AsyncSocketException(
                    AsyncSocketException::INTERNAL_ERROR,
                    "connect server failed",
                    errno);
        }
    }

}

void AsyncSocket::setReadCB(ReadCallback* readCB)
{
    if(readCB == NULL)
    {
        readCallback_ = NULL;//cancel callback
        return;
    }

    if(readCB == readCallback_)
    {
        return;
    }
    
    //regist io event 
    switch(state_)
    {
    case StateEnum::CONNECTING:
    case StateEnum::FAST_OPEN:
       readCallback_ = readCB;//连接过程中接收回调设置
       return;
    case StateEnum::ESTABLISHED:
    {
       readCallback_ = readCB;
       int16_t oldFlag = eventFlags_;
       if(readCallback_)
       {
           eventFlags_ |= EventHandler::READ;
       }
       else
       {
           eventFlags_ &= ~EventHandler::READ;
       }
       if(oldFlag != eventFlags_)
       {
           (void)updateEventRegistration();
       }
       return;
    }
    case StateEnum::CLOSED:
    case StateEnum::ERROR:
       assert(false);
       return invalidState(readCB);
    case StateEnum::UNINT:
       return invalidState(readCB);
    }
}

void AsyncSocket::ioReady(int16_t event)
{
    std::cout<<"AsyncSocket::ioReady is running!"<<std::endl;
}

bool AsyncSocket::updateEventRegistration()
{
    assert(eventBase_->isInEventBaseThread());
    if(eventFlags_ == EventHandler::NONE)
    {
        ioHandler_.unregisterHandler();
    }
    if(!ioHandler_.registHandler(eventFlags_))
    {
        eventFlags_ = EventHandler::NONE;
        std::cout<<"updateEventRegistration event flags failed"<<std::endl;
        return false;
    }
    return true;
}

void AsyncSocket::invalidState(ReadCallback* callback)
{
}

void AsyncSocket::invalidState(ConnectCallback* callback)
{
}

void AsyncSocket::failConnect(const AsyncSocketException& ex)
{
    if(connectCallback_)
    {
        connectCallback_->connectErr(ex);
    }
}

} //libext
