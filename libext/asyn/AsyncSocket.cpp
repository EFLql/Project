#include <libext/asyn/AsyncSocket.h>
#include <assert.h>
#include <iostream>

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

AsyncSocket::~AsyncSocket()
{
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

} //libext
