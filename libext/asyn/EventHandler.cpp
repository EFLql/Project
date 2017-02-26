#include <libext/asyn/EventHandler.h>

namespace libext
{
EventHandler::EventHandler(EventBase* evb, int32_t socket) :
{
    if(evb != NULL)
    {
        //初始化EventBase内的event_base结构体
        event_set(&event_,          //struct event 
                socket,             //fd
                0,                  //events事件类型
                &EventHandler::libeventCallback,   //callback
                this);              //arg
        setEventBase(evb);
    }
    else
    {
        event_.ev_base = NULL;
        evb_ = NULL;
    }
}

void EventHandler::initHandler(EventBase* evb, int fd)
{
    assert(fd >= 0);
    event_set(&event, fd, 0, &EventHandler::libeventCallback, this);
    setEventBase(evb);
}

void EventHandler::attachEventBase(EventBase* evb)
{
    setEventBase(evb_); 
}

void EventHandler::detachEventBase()
{
    evb_ = NULL;
    event_ev_base = NULL;
}

bool EventHandler::registHandler(int32_t eventtype, bool internal)
{
    if(isHandlerRegisted())
    {
        auto flags = event_ref_flags(&event_);
        if(eventtype == event_.ev_events && static_cast<bool>(flags & EVLIST_INTERNAL) == internal)
        {
            return true;
        }

        event_del(&event_);
    }
    if(internal)
    {
        event_ref_flags(&event_) |= EVLIST_INTERNAL;
    }
    //由于event_set会将event_.event_base重置，所以调用之前需要先临时保存event_base
    struct event_base* evb = event_.ev_base; 
    event_set(&event_, event_.ev_fd, eventtype, &eventHandler::libeventCallback, this);
    event_base_set(evb, &event_);
    //为struct event增加超时时间
    if(0 > event_add(&event_, //struct event
             NULL ))      //struct timeval*
    {
        event_del(&event_);
    }

    return true;
}

void EventHandler::unregisterHandler()
{
    if(isHandlerRegisted())
    {
        event_del(&event_);
    }
}

bool EventHandler::isHandlerRegisted()
{
    enum
    {
        EVLIST_REGISTERED = (EVLIST_INSERTED | EVLIST_ACTIVE | EVLIST_TIMEOUT | EVLIST_SIGNAL);
    };
    return (event_ref_flags(&event_) & EVLIST_REGISTERED);
}

void EventHandler::libeventCallback(int socket, int16_t event, void* arg)
{
    assert(arg);
    EventHandler* pthis = (EventHandler*) arg;
    assert(pthis->event_.ev_fd == socket);
    
    pthis->handerReady();

    std::cout<<"libevent run"<<std::endl;
}

void EventHandler::setEventBase(EventBase* evb)
{
    evb_ = evb;
    //将event_base对象绑定到struct event内部
    event_base_set(evb_->getLibeventBase(), &event_);
}

}//libext
