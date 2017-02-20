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
    AsyncServerSocket(libext::EventBase* env);
    ~AsyncServerSocket();

    int createSocket();
    void bind(libext::SocketAddr);
    void listen(int backlog);
    void startAccepting();
    void addAcceptCB(AcceptCallback* callback, libext::EventBase* env);
    void removeAcceptCB(AcceptCallback* callback, libext::EventBase* env);
    void setReusePortEnable(bool reuse);
    EventBase* getEventBase()
    {
        return env_;
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
        void start(libext::EventBase* env, int maxInQueue);
        void stop(libext::EventBase* env); 
        virtual void messageAvailable(QueueMessage msg);
    private:
        AcceptCallback* callback_;
        ConnectionEventCallback connectionEventCallback_;
        NotificationQueue<QueueMessage> queue_;
    };
   
    //lql-need modify
    struct CallbackInfo
    {
        CallbackInfo(AcceptCallback* tcallback, libext::EventBase* tenv) 
            : callback(tcallback), env(tenv){}
        AcceptCallback* callback;
        EventBase* env;
        RemoteAcceptor* consumer;
    };

    class ServerEventHandler : public libext::EventHandler
    {
    public:
        ServerEventHandler(libext::EventBase* env, int socket, AsyncServerSocket* parent) :
            libext::EventHandler(env, socket),
            socket_(socket),
            env_(env),
            parent_(parent)
        {}

        int socket_;
        AsyncServerSocket* parent_;
        libext::EventBase* env;
    };
private:
    libext::EventBase* env_;
    std::vector<ServerEventHandler> sockets_;
    std::vector<CallbackInfo> callbacks_;
};

}//libext
