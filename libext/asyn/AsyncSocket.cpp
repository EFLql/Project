#include <libext/asyn/AsyncSocket.h>
#include <glog/logging.h>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

namespace libext
{
//server端
AsyncSocket::AsyncSocket(EventBase* evb)
: eventBase_(evb)
, maxReadsPerEvent_(16)
, readCallback_(NULL)
, fd_(-1)
, ioHandler_(evb, 0, this)
, eventFlags_(EventHandler::NONE)
{
}

AsyncSocket::AsyncSocket(EventBase* evb, int fd)
: maxReadsPerEvent_(16)
, readCallback_(NULL)
, fd_(fd)
, eventBase_(evb)
, ioHandler_(evb, fd, this)
, eventFlags_(EventHandler::NONE)
{
    //服务端使用，在AsyncSocket构造之前
    //和客户端的连接就已经建立了，所以
    //这里初始时状态就是连接建立成功了的
    state_ = StateEnum::ESTABLISHED;
}

//客户端
AsyncSocket::AsyncSocket::AsyncSocket()
: maxReadsPerEvent_(16)
, readCallback_(NULL)
, state_(StateEnum::UNINT)
, ioHandler_(NULL, 0, this)
, eventFlags_(EventHandler::NONE)
{

}

AsyncSocket::AsyncSocket(EventBase* evb, const SocketAddr& addr,
            uint32_t connectTimeout)
    :AsyncSocket(evb)

{
    connect(NULL, addr, connectTimeout);
}

AsyncSocket::AsyncSocket(EventBase* evb, const std::string& ip,
            uint16_t port, uint32_t connectTimeout)
    :AsyncSocket(evb)
{
    connect(NULL, SocketAddr(ip, port), connectTimeout);
}


AsyncSocket::~AsyncSocket()
{
}

void AsyncSocket::connect(ConnectCallback* callback, 
        const SocketAddr& addr, uint32_t connectTimeout) noexcept
{
    assert(eventBase_->isInEventBaseThread());
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
        //lql-note 下面的异常情况发生的时候需要关闭掉
        //fd_，现在暂时没有关闭!!!
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
        if(socketConnect(addr) < 0)
        {
            return;//连接正在异步进行中
        }
        
    } catch (const AsyncSocketException& ex)
    {
        failConnect(__func__, ex);
    }
    
    std::cout<<"AsyncSocket connect successfuly immediately"<<std::endl; 
    state_ = StateEnum::ESTABLISHED;
    invokeConnectSuccess();
}

void AsyncSocket::connect(ConnectCallback* callback, const std::string& ip,
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
        failConnect(__func__, tex);
    }
}

int AsyncSocket::socketConnect(const SocketAddr& addr)
{
    assert(fd_ != -1);
    sockaddr_in addrsvr = addr.getSocketAddr();
    int ret = ::connect(fd_, 
                        (struct sockaddr*)&addrsvr,
                        sizeof(addr.getSocketAddr()));
    if(ret < 0)
    {
        if(errno == EINPROGRESS)//异步模式下连接正在进行
        {
            registerForConnectEvents();
            return ret;
        }
        else
        {
            throw AsyncSocketException(
                    AsyncSocketException::INTERNAL_ERROR,
                    "connect failed(immediately)",
                    errno);
        }
    }
    return ret;
}

void AsyncSocket::registerForConnectEvents()
{
    assert(eventFlags_ == EventHandler::NONE);

    eventFlags_ = EventHandler::WRITE;
    if(!ioHandler_.registHandler(eventFlags_))
    {
        throw AsyncSocketException(
                AsyncSocketException::INTERNAL_ERROR,
                "failed to register AsyncSocket connect handelr");
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
    std::cout<<"AsyncSocket::ioReady() this"<<this<< \
        ", fd= "<<fd_<<", events="<<event<< \
        ", state= "<<int(state_)<<std::endl;
    
    assert(event & EventHandler::READ_WRITE);
    assert(eventBase_->isInEventBaseThread());
    
    int relevantEvents = event & EventHandler::READ_WRITE;
    if(relevantEvents == EventHandler::READ)
    {
        handleRead();
    }
    else if(relevantEvents == EventHandler::WRITE)
    {
        handleWrite();
    }
    else if(relevantEvents == EventHandler::READ_WRITE)//这种情况什么情形下发生?
    {
        //code..
    } 
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

bool AsyncSocket::updateEventRegistration(int32_t enable,
                                          int32_t disable)
{
    int32_t oldFlags = eventFlags_;
    eventFlags_ |= enable;
    eventFlags_ &= ~disable;
    if(oldFlags == eventFlags_)
    {
        return true;
    }
    return updateEventRegistration();
}

void AsyncSocket::invalidState(ReadCallback* callback)
{
}

void AsyncSocket::invalidState(ConnectCallback* callback)
{
}

void AsyncSocket::failConnect(const char* fn, const AsyncSocketException& ex)
{
    std::cout<<"AsyncSocket (this= "<<this<<"state= "
        <<int(state_)<<" host= "<<
        addr_.getSocketAddr().sin_addr.s_addr
        <<") failed while connecting "<<ex.what();
    if(connectCallback_)
    {
        connectCallback_->connectErr(ex);
    }
}

void AsyncSocket::failRead(const char* fn, const AsyncSocketException& ex)
{
}

void AsyncSocket::failWrite(const char* fn, const AsyncSocketException& ex)
{
}

void AsyncSocket::invokeConnectSuccess()
{
    if(connectCallback_)
    {
        ConnectCallback* callback = connectCallback_;
        connectCallback_ = NULL;
        callback->connectSucc();
    }
}

void AsyncSocket::prepareReadBuffer(void** buff, size_t* bufflen) noexcept
{
    CHECK(readCallback_);
    readCallback_->getReadBuff(buff, bufflen);
}

AsyncSocket::ReadResult
AsyncSocket::performRead(void** buff, size_t* bufflen, size_t* /*offset*/) noexcept
{
    std::cout<<"AsyncSocket::performRead() this= "<<this<<
        "buff= "<<*buff<<" bufflen= "<<*bufflen<<std::endl;
    
    int recvFlags = 0;
    ssize_t bytes = recv(fd_, *buff, *bufflen, MSG_DONTWAIT | recvFlags);
    if(bytes < 0)
    {
       if(errno == EAGAIN || errno == EWOULDBLOCK)
       {
           return ReadResult(READ_BLOCKING);
       }
       else
       {
           return ReadResult(READ_ERROR);
       }
    }

    return ReadResult(bytes);
}

void AsyncSocket::handleRead() noexcept
{
    assert(state_ == StateEnum::ESTABLISHED);
    assert(eventFlags_ & EventHandler::READ);
    assert(readCallback_ != NULL);

    uint16_t numReads = 0;
    EventBase* originalEventBase = eventBase_;
    while(readCallback_ && originalEventBase == eventBase_)
    {
        void* buff = NULL;
        size_t bufflen = 0, offset = 0;
        try
        {
            prepareReadBuffer(&buff, &bufflen);
            std::cout<<"prepareReadBuffer() buff= "<<buff<<
                " bufflen= "<<bufflen<<std::endl;
        } catch(const AsyncSocketException& ex)
        {
            return failRead(__func__, ex);
        } catch(const std::exception& ex)
        {
            AsyncSocketException tex(
                   AsyncSocketException::BAD_ARGS,
                   std::string("ReadCallback::getReadBuffer() "
                       "throw exception") + 
                   ex.what());
           return failRead(__func__, tex);
        } catch(...)
        {
           AsyncSocketException tex(
                  AsyncSocketException::BAD_ARGS,
                  "ReadCallback::getReadBuffer() throw "
                  "a non-type exception");
          return failRead(__func__, tex);
        }
        if(buff == NULL && bufflen == 0)
        {
            AsyncSocketException ex(
                    AsyncSocketException::BAD_ARGS,
                    "ReadCallback::getReadBuffer() return "
                    "a empty buffer");
            return failRead(__func__, ex);
        }

        //开始读取数据,performRead这个函数
        //不拋异常，异常信息通过返回值返回
        //因为有些读取错误并不是异常,属于正常情况
        //只有真正有异常时才调用回调通知异常发生
        auto readResult = performRead(&buff, &bufflen, &offset);
        auto bytesRead = readResult.readReturn;
        
        std::cout<<"this= "<<this<<" ,AsyncSocket::handleRead() got "<<
            bytesRead<<" bytes"<<std::endl;

        if(bytesRead > 0)
        {
            readCallback_->readDataAvailable(bytesRead);
            if(size_t(bytesRead) < bufflen)//数据全部读取完毕，退出
            {
                return;
            }
        }
        else if(bytesRead == READ_BLOCKING)//socket内无数据可读
        {
            return;
        }
        else if(bytesRead == READ_ERROR)//读取出错
        {
            if(readResult.exception)
            {
                return failRead(__func__, *(readResult.exception));
            }

            AsyncSocketException ex(
                    AsyncSocketException::INTERNAL_ERROR,
                    "recv() failed", errno);
            return failRead(__func__, ex);
        }
        else//read eof of socket
        {
           assert(bytesRead == READ_EOF);
           if(!updateEventRegistration(0, EventHandler::READ))
           {
               return;
           }
           ReadCallback* callback = readCallback_;
           readCallback_ = NULL;
           callback->readEOF();
           return;
        }
        if(maxReadsPerEvent_ && (++numReads >= maxReadsPerEvent_))
        {
            return;
        }
    }
}

void AsyncSocket::handleWrite() noexcept
{
    //async connect inprogress
    if(state_ == StateEnum::CONNECTING)
    {
        return handleConnect();
    }

    assert(state_ == StateEnum::ESTABLISHED);
    //Normal write
    //当有数据待发送的时候才注册EentHandler::WRITE
    assert(writeReqHead_ != NULL);

    EventBase* originalEventBase = eventBase_;
    while(writeReqHead_ != NULL && originalEventBase == eventBase_)
    {
        WriteResult writeResult = writeReqHead_->performWrite();
        //lql-note code for write result
        if(writeResult.writeReturn < 0)//write error
        {
        }
    }
}

void AsyncSocket::handleConnect() noexcept
{
    assert(state_ == StateEnum::CONNECTING);
    assert(eventFlags_ == EventHandler::WRITE);
    eventFlags_ = EventHandler::NONE;

    int error;
    socklen_t len = sizeof(error);
    //调用getsockopt判断连接是否成功
    int rv = getsockopt(fd_, SOL_SOCKET, SO_ERROR, 
            &error, &len);
    if(rv == -1)//error
    {
        AsyncSocketException tex(
                AsyncSocketException::INTERNAL_ERROR,
                "getsockopt failed on AsyncSocket::fd_",
                errno);
        return failConnect(__func__, tex);
    }

    if(error != 0)//async connect failed
    {
        AsyncSocketException tex(
                AsyncSocketException::NOT_OPEN,
                "AsyncSocket connect failed",
                error);
        return failConnect(__func__, tex);
    }

    std::cout<<"AsyncSocket "<<this<<", fd= "<<fd_<<
        " successfuly connected"<<std::endl;
    //async connect succ
    state_ = StateEnum::ESTABLISHED;
    invokeConnectSuccess();

    handleInitialReadWrite();
}

void AsyncSocket::handleInitialReadWrite() noexcept
{
    if(readCallback_ && !(eventFlags_ & EventHandler::READ))
    {
        assert(state_ == StateEnum::ESTABLISHED);
        if(!updateEventRegistration(EventHandler::READ, 0))
        {
            return;
        }
    }
    else if(readCallback_ == NULL)
    {
        //unregist read event
        updateEventRegistration(0, EventHandler::READ);
    }

    //lql-note code continue... for write request pending
}

} //libext
