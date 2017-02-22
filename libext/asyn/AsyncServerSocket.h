#pragma once
#include "Acceptor.h"
#include "EventBase.h"
#include "EventHandler"

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <vector>

namespace libext
{
class AsyncServerSocket
{
public:
    AsyncServerSocket();
    AsyncServerSocket(libext::EventBase* evb);
    ~AsyncServerSocket();

    int createSocket();
    void bind(libext::SocketAddr);
    void listen(int backlog);
    void startAccepting();
    void addAcceptCB(AcceptCallback* callback, libext::EventBase* evb);
    void removeAcceptCB(AcceptCallback* callback, libext::EventBase* evb);
    void setReusePortEnable(bool reuse);
    EventBase* getEventBase()
    {
        return evb_;
    }

    //lql-need modify
    class ConnectionEventCallback
    {
    };

    //lql-need modify
    class AcceptCallback
    {
    };
    //lql-need modify
    class RemoteAcceptor : public libext::NotificationQueue<QueueMessage>::consumer
    {
    public:
        void start(libext::EventBase* evb, int maxInQueue);
        void stop(libext::EventBase* evb); 
        virtual void messageAvailable(QueueMessage msg);
    private:
        AcceptCallback* callback_;
        ConnectionEventCallback connectionEventCallback_;
        NotificationQueue<QueueMessage> queue_;
    };
   
    //lql-need modify
    struct CallbackInfo
    {
        CallbackInfo(AcceptCallback* tcallback, libext::EventBase* tevb) 
            : callback(tcallback), evb(tevb){}
        AcceptCallback* callback;
        EventBase* evb;
        RemoteAcceptor* consumer;
    };

    class ServerEventHandler : public libext::EventHandler
    {
    public:
        ServerEventHandler(libext::EventBase* evb, int socket, AsyncServerSocket* parent) :
            libext::EventHandler(evb, socket),
            socket_(socket),
            evb_(evb),
            parent_(parent)
        {}
        void handerReady() override;
        int socket_;
        AsyncServerSocket* parent_;
        libext::EventBase* evb_;
    };
private:
    libext::EventBase* evb_;
    std::vector<ServerEventHandler> sockets_;
    std::vector<CallbackInfo> callbacks_;
};

}//libext
