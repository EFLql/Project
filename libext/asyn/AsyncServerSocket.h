#pragma once
#include <libext/asyn/AsyncSocketBase.h>
#include <libext/asyn/EventBase.h>
#include <libext/asyn/EventHandler.h>
#include <libext/SocketAddr.h>
#include <libext/asyn/NotificationQue.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <vector>
#include <exception>

namespace libext
{
class AsyncServerSocket : public AsyncSocketBase
{
public:
    AsyncServerSocket();
    AsyncServerSocket(libext::EventBase* evb);
    //防止浅拷贝
    AsyncServerSocket(AsyncServerSocket&&) = delete;
    AsyncServerSocket(AsyncServerSocket) = delete;
    AsyncServerSocket operator = (AsyncServerSocket) = delete;
    ~AsyncServerSocket();
    enum MessageType
    {
        MSG_NEW_CONN,
        MSG_ERROR
    };
    struct QueueMessage
    {
        MessageType type;
        int socket;
        SocketAddr addr;
    };
    //lql-need modify
    class ConnectionEventCallback
    {
    public:
        ConnectionEventCallback() {}
        ~ConnectionEventCallback() {}
    };

    //lql-need modify
    class AcceptCallback
    {
    public:
        AcceptCallback() {}
        virtual ~AcceptCallback() = default;

        virtual void connectionAccepted(int fd,
                                        const SocketAddr& clientAddr) = 0;
        virtual void acceptError(const std::exception& e) = 0;
        virtual void acceptStarted() {}
    };
    //lql-need modify
    class RemoteAcceptor : public libext::NotificationQueue<QueueMessage>::Consumer
    {
    public:
        RemoteAcceptor(AcceptCallback* callback, ConnectionEventCallback* connectionEventCallback)
            :callback_(callback),
            connectionEventCallback_(connectionEventCallback){}
        void start(libext::EventBase* evb, int maxInQueue);
        void stop(libext::EventBase* evb); 
        void messageAvailable(QueueMessage&& msg) override;
        NotificationQueue<QueueMessage>* getQueue()
        {
            return &queue_;
        }
    private: 
        AcceptCallback* callback_;
        ConnectionEventCallback* connectionEventCallback_;
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


    int  createSocket();
    bool setupSocket(int family, int fd)
    bool bind(libext::SocketAddr& addr);
    bool listen(int backlog);
    void startAccepting();
    void addAcceptCB(AcceptCallback* callback, libext::EventBase* evb);
    void removeAcceptCB(AcceptCallback* callback, libext::EventBase* evb);
    void setReusePortEnable(bool reuse);
    void handlerReady(); 
    EventBase* getEventBase() const override
    {
        return evb_;
    }

private:
    static const int32_t kDefaultMessagesInQueue = 1024;    
    class ServerEventHandler : public libext::EventHandler
    {
    public:
        ServerEventHandler(libext::EventBase* evb, int socket, AsyncServerSocket* parent) :
            libext::EventHandler(evb, socket),
            socket_(socket),
            evb_(evb),
            parent_(parent)
        {}
        void handlerReady() override
        {
            parent_->handlerReady();
        }
        int socket_;
        AsyncServerSocket* parent_;
        libext::EventBase* evb_;
    };
private:
    void dispatchSocket(int socket, const SocketAddr& addr);
    void dispatchError(int socket, const SocketAddr& addr);
    CallbackInfo* nextCallback()
    {
        callbackIndex_ = callbackIndex_ % callbacks_.size(); 
        return &callbacks_[callbackIndex_];
    }
private:
    libext::EventBase* evb_;
    std::vector<ServerEventHandler> sockets_;
    std::vector<CallbackInfo> callbacks_;
    int callbackIndex_{0};
    int32_t maxInQueue_;
    bool accepted_;
    bool reusePort_;
};

}//libext
