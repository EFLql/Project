#include "AsyncServerSocket.h"

using libext::AsyncServerSocket;

AsyncServerSocket::AsyncServerSocket()
{
}

AsyncServerSocket::AsyncServerSocket(libext::EventBase* evb) :
evb_(evb)
{
}

int AsyncServerSocket::createSocket()
{
    int fd;
    if( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        return -1;
    }

    return fd;
}

bool AsyncServerSocket::bind(libext::SocketAddr& addr)
{
    int fd;
    if(socket_.size() == 0)
    {
        fd = createSocket();
    }
    else if(socket_.size() == 1)
    {
        fd = socket_[0].socket_;
    }
    else
    {
        //LOG-ERR Asyncserversocket只能监听一个socket
        return false;
    }
    int ret = ::bind(fd, (struct socketaddr*)addr.getSocketAddr(), sizeof(struct socketaddr));
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
    if(socket_.size() == 1)
    {
        fd = socket_[0].socket_;
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
    if(callbacks_.empty())
    {
        //LOG_INFO 没有安装任何的回调
        return;
    }

    //对每个eventhandler注册事件
    for(auto& handler : socket_)
    {
        //为serversocket注册事件，后续socket的所有事件均会通知到hander
        handler.registHandler(EventHandler::READ | EventHandler::PERSIST, false);
    }
}

void AsyncServerSocket::addAcceptCB(AcceptCallback* callback, libext::EventBase* evb)
{
    if(!evb_)
    {
        evb = evb_;
    }

    callbacks_.push_back(callback, evb); 
    RemoteAcceptor acceptor = new RemoteAcceptor(callback, evb);
    acceptor->start(evb, maxInQueue);
}

void AsyncServerSocket::removeAcceptCB(AcceptCallback* callback, libext::EventBase* evb)
{
    if(callbacks_.empty()) return;

    std::vector<CallbackInfo>::iterator itr = callbacks_.begin();
    while(itr != callbacks_.end())
    {
        if(itr.callback == callback && itr.evb == evb)
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
        if(setsocketopt(socket.socket_, SOL_SOCKET,SO_REUSEPORT, 
              &val, sizeof(val)) != 0)
        {
            //LOG_ERR 设置socket选项失败
        }
    }    
}

//////////////////////////////////////
void AsyncServerSocket::RemoteAcceptor::start(libext::EventBase* evb, int maxInQueue)
{//lql-伪代码记录流程，后续需要修改
    //初始化自己的队列,当有连接请求时才能塞到其任务队列进行消费
    queue_.setMaxSize(maxInQueue);

    //往队列里面投递任务
    evb->runInEventBase([](){
            //开始消费queue_里面的任务
            this->startConsume(evb, &queue_);
            });
}

void AsyncServerSocket::RemoteAcceptor::stop(libext::EventBase* evb)
{

}

void AsyncServerSocket::RemoteAcceptor::messageAvailable(QueueMessage msg)
{
}
