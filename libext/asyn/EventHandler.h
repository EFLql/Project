#pragma once

namespace libext
{
    
class EventHandler
{
public:
    enum EventFlags
    {
        NONE = 0,
        READ = EV_READ,
        WRITE = EV_WRITE,
        READ_WRITE = EV_READ | EV_WRITE,
        PERSIST = EV_PERSIS,
    };
    EventHandler(EventBase* evb, int socket);
    virtual ~EventHandler() {}

    void initHandler(EventBase* evb, int fd);
    void setEventBase(EventBase* evb);
    void attachEventBase(EventBase* evb);
    void dettachEventBase(EventBase* evb);
    void registHandler(int32_t eventtype);
    bool EventHandler::isHandlerRegisted();
    void EventHandler::unregisterHandler();
    void libeventCallback(int socket, uint32_t event, void* arg); 
    virtual void handlerReady() = 0; 
private:
    EventBase* evb_;
    struct event event_;
    int fd_;
};


}//libext
