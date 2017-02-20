#include "EventBase.h"

namespace libext
{
EventBase::EventBase() : 
bstop_(false)
{
    base_ = event_base_new();
}

EventBase::~EventBase()
{
}

void EventBase::loop()
{
}

void EventBase::loopForever()
{
    loopBody();
}

bool EventBase::runInEventBaseThread(Func fun)
{
    if(!fun)
    {
        //LOG_ERR
        return false;
    }

    queue_.putMessage(fun);
}

void EventBase::loopBody()
{
    pid_ = pthread_self();
    while(!bstop_)
    {

        event_base_loop(base_, EVLOOP_NONBLOCK);
    }
}

bool EventBase::isInEventBaseThread()
{
    if(pthread_self() == pid_) return true;
    
    return false;
}
}//libext
