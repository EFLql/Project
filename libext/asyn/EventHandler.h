#pragma once
#include <libext/typedef.h>
#include <libext/asyn/EventBase.h>
#include <event.h>

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
        PERSIST = EV_PERSIST,
    };
    EventHandler(EventBase* evb = NULL, int socket = 0);
    virtual ~EventHandler() {}

    void initHandler(EventBase* evb, int fd);
    void setEventBase(EventBase* evb);
    void attachEventBase(EventBase* evb);
    void detachEventBase();
    bool registHandler(int32_t eventtype, bool internal = false);
    bool isHandlerRegisted();
    void unregisterHandler();
    static void libeventCallback(int socket, int16_t event, void* arg); 
    virtual void handlerReady() = 0; 
private:
    EventBase* evb_;
    struct event event_;
};


}//libext
