#include <libext/asyn/AsyncServerSocket.h>

namespace libext
{
AsyncServerSocket::AsyncServerSocket()
: maxInQueue_(kDefaultMessagesInQueue)
, accepted_(false)
, reusePort_(false)
{
}

AsyncServerSocket::AsyncServerSocket(libext::EventBase* evb) :
evb_(evb)
{
}

AsyncServerSocket::~AsyncServerSocket()
{
}

int AsyncServerSocket::createSocket()
{
    int fd;
    if( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cout<<"createSocket faild "<<strerror(errno)<<std::endl;
        throw std::logic_error("createSocket faild");
    }
    try
    {
        setupSocket(AF_INET, fd)
    }catch(...)
    {
        std::cout<<"setupSocket faild"<<std::endl;
        closeNoInt(fd);
        throw;
    }
    return fd;
}

void AsyncServerSocket::setupSocket(int family, int fd)
{
    //设置socket为非阻塞
    if(!fcntl(fd, F_SETFL, O_NONBLOCK))
    {
        throw std::exception("setupSocket fcnt faild to set socket to non-bloking");
    }
    
    //设置端口可重用，防止2MSL延迟阻止服务重启
    int one = 1;
    int zero = 0;
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) != 0)
    {
        //不是致命错误，只打印错误日志继续运行
        std::cout<<"setupSocket faild to set SO_REUSEADDR on asyn socket" \
            <<strerror(errno)<<std::endl;
    }
   
    //端口重用，支持多线程多个accept线程 
    if(reusePort_ && 
           setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one)) != 0)
    {
        std::cout<<"setupSocket faild to set SO_REUSEPORT on asyn socket" \ 
            <<strerror(errno)<<std::endl;
    }

    //set keepAlive
    if(setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, 
                keepAlive_ ? one : zero, sizeof(int) != 0))
    {
        std::cout<<"setupSocket faild to set SO_KEEPALIVE on asyn socket" \ 
            <<strerror(errno)<<std::endl;
    }
#ifndef TCP_NOPUSH
    if(family != AF_UNIX)
    {
        //TCP_NODELAY禁用Nagle算法，使得数据能够快速的发送出去，但是
        //当每次发送的包很小是则可能引起网络拥塞,比如发送的内容就一个字节
        //但是TCP/IP协议层会构造40字节的头数据，造成负载过大,Nagle算法能解决这种问题
        //但是Nagle在发送大数据的时候会造成副作用
        //https://en.wikipedia.org/wiki/Nagle's_algorithm
        //这里禁用Nagle算法，必须确保一次传输的数据量比较大，否则会造成网络拥塞
        if(setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one)) != 0)
        {
            std::cout<<"setupSocket faild to set TCP_NODELAY on asyn socket" \
                <<strerror(errno)<<std::endl;
        }
    }
#endif
  
//开启tcp fast open选项 
#ifdef LIBEXT_ALLOW_TFO
#endif 
            
}

bool AsyncServerSocket::bind(libext::SocketAddr& addr)
{
    int fd;
    if(sockets_.size() == 0)
    {
        fd = createSocket();
        sockets_.push_back(ServerEventHandler(evb_, fd, this));
    }
    else if(sockets_.size() == 1)
    {
        fd = sockets_[0].socket_;
    }
    else
    {
        //LOG-ERR Asyncserversocket只能监听一个socket
        return false;
    }
    struct sockaddr_in saddr = addr.getSocketAddr();
    int ret = ::bind(fd, (struct sockaddr*)&saddr, sizeof(struct sockaddr));
    if(ret == -1)
    {
        //LOG_ERR
        std::cout<<strerror(errno)<<std::endl;
        return false;
    }

    return true;    
}

bool AsyncServerSocket::listen(int32_t backlog)
{
    int fd;
    if(sockets_.size() == 1)
    {
        fd = sockets_[0].socket_;
    }
    else
    {
        //LOG-ERR Asyncserversocket只能监听一个socket
        return false;
    }

    int ret = ::listen(fd, backlog);
    if(ret == -1)
    {
        //LOG_ERR
        std::cout<<strerror(errno)<<std::endl;
        return false;
    }

    return true;    

}

void AsyncServerSocket::startAccepting()
{
    assert(evb_ == NULL || evb_->isInEventBaseThread());//当前肯定在evb循环里面

    accepted_ = true;
    if(callbacks_.empty())
    {
        //LOG_INFO 没有安装任何的回调
        return;
    }

    //对每个eventhandler注册事件
    for(auto& handler : sockets_)
    {
        //为serversocket注册事件，后续socket的所有事件均会通知到hander
        handler.registHandler(EventHandler::READ | EventHandler::PERSIST);
    }
}

void AsyncServerSocket::addAcceptCB(AcceptCallback* callback, libext::EventBase* evb)
{
    RemoteAcceptor* acceptor;
    try
    {
        //new在内存紧张的情况下会拋异常
        acceptor = new RemoteAcceptor(callback, NULL);//lql-mark,
        acceptor->start(evb, maxInQueue_);
    }catch(const std::exception& e)
    {
        delete acceptor;
        throw;
    }
    bool shouldStartAccept = accepted_ && callbacks_.empty();
    callbacks_.push_back(CallbackInfo(callback, evb));
    callbacks_.back().consumer = acceptor;
    if(shouldStartAccept)
    {
        startAccepting();
    }
}

void AsyncServerSocket::removeAcceptCB(AcceptCallback* callback, libext::EventBase* evb)
{
    if(callbacks_.empty()) return;

    std::vector<CallbackInfo>::iterator itr = callbacks_.begin();
    while(itr != callbacks_.end())
    {
        if(itr->callback == callback && itr->evb == evb)
        {
            callbacks_.erase(itr);
            break;
        }
        ++itr;
    }
}

void AsyncServerSocket::setReusePortEnable(bool reuse)
{
    for(auto& socket : sockets_)
    {
        int val = reuse ? 1 : 0;
        if(setsockopt(socket.socket_, SOL_SOCKET,SO_REUSEPORT, 
              &val, sizeof(val)) != 0)
        {
            //LOG_ERR 设置socket选项失败
        }
    }
    reusePort_ = reuse; 
}

void AsyncServerSocket::handlerReady()
{
    SocketAddr addr;
    socklen_t addrlen = sizeof(addr.getSocketAddr());
    struct sockaddr_in saddr = addr.getSocketAddr();
    int clientSocket = accept(sockets_[0].socket_, 
            (struct sockaddr*)&saddr, &addrlen);
    
    if(clientSocket >= 0)
    {
        //constrol

        dispatchSocket(clientSocket, addr);
    }
    else
    {
        //constrol....
        //

        dispatchError(clientSocket, addr);
    }
}

void AsyncServerSocket::dispatchSocket(int socket, const SocketAddr& addr)
{
    CallbackInfo* info = nextCallback();//调度处理线程
    

    QueueMessage msg;
    msg.type = MSG_NEW_CONN;
    msg.socket = socket;
    msg.addr = addr;
    
    while(true)
    {
        CHECK(info);
        CHECK(info->consumer);
        
        if(info->consumer->getQueue()->tryPutMessage(msg))
        {
            return;
        }

        info = nextCallback();
    }
}

void AsyncServerSocket::dispatchError(int socket, const SocketAddr& addr)
{
    CallbackInfo* info = nextCallback();//调度处理线程
    

    QueueMessage msg;
    msg.type = MSG_ERROR;
    msg.socket = socket;
    msg.addr = addr;
    
    while(true)
    {
        CHECK(info);
        CHECK(info->consumer);
        
        if(info->consumer->getQueue()->tryPutMessage(msg))
        {
            return;
        }

        info = nextCallback();
    }
}

//////////////////////////////////////
void AsyncServerSocket::RemoteAcceptor::start(libext::EventBase* evb, int maxInQueue)
{
    //初始化自己的队列,当有连接请求时才能塞到其任务队列进行消费
    queue_.setMaxQueueSize(maxInQueue);

    //往队列里面投递任务
    if(!evb->runInEventBaseThread([this, evb](){
            //开始消费queue_里面的任务
            this->startConsuming(evb, &queue_);
            }))
    {
        throw std::invalid_argument("Unable start waiting on accept notification  \
               queue in specifical eventbase"); 
    }
}

void AsyncServerSocket::RemoteAcceptor::stop(libext::EventBase* evb)
{

}

void AsyncServerSocket::RemoteAcceptor::messageAvailable(QueueMessage&& msg)
{
    std::cout<<"Accepted client socket= "<<msg.socket<<std::endl;
}

} //libext
