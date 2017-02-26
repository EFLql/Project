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
    
    if(isRuningInEventBase())
    {
        runInLoop(fun);
        return true;
    }
    queue_.putMessage(fun);
    
    return true;
}

void EventBase::runInLoop(Func fun)
{
    SpinLockGuard g(spinLock_);
    callbacks_.push_back(fun);
}

void EventBase::loopBody()
{
    spinLock_.lock();
    pid_ = pthread_self();
    spinLock_.unlock();
    
    while(true)
    {
        spinLock_.lock();
        if(bstop_)
        {
            spinLock_.unlock();
            break;
        }
        spinLock_.unlock();

        event_base_loop(base_, EVLOOP_NONBLOCK);
        runCallback();
    }

    spinLock_.lock();
    pid_ = NULL;
    bstop_ = false;
    spinLock_.unlock();
}

void EventBase::runCallback()
{
    std::vector<Func> tcallback;
    
    spinLock_.lock();
    tcallback.swap(callbacks_);
    spinLock_.unlock();

    for(auto itr : tcallback)
    {
        *itr();
    }
}

bool EventBase::isRunningEventBase()
{
    int r = pthread_equal(pid_, 0);
    
    return static_cast<bool>(r);
}

bool EventBase::isInEventBaseThread()
{
    int r = pthread_equal(pthread_self() == pid_)
    
    return static_cast<bool>(r);
}

void EventBase::terminateLoop()
{
    spinLock_.lock();
    bstop_ = true;
    spinLock_.unlock();

    event_base_loopbreak(base);
}

}//libext
