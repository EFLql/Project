#include <libext/asyn/EventBase.h>
#include <libext/asyn/NotificationQue.h>
namespace libext
{

class FunctionRunner : public NotificationQueue<libext::Func>::Consumer
{
public:
    void messageAvailable(libext::Func&& f) override
    {
        if(!f)
        {
            return;
        }
        try
        {
            f();
        }catch(const std::exception& e)
        {
            std::cout<<"Unable handler a unknow exception "<<e.what()<<std::endl;
        }
    } 
};

void EventBase::startConsumingMsg()
{
    assert(fnRunner_.get());
    fnRunner_->stopConsuming();
    fnRunner_->startConsuming(this, queue_.get()); 
}

EventBase::EventBase() 
: stop_(false)
{
    base_ = event_base_new();
    initNotificationQueue();
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

void EventBase::loopOnce()
{
    loopBody(true);
}

bool EventBase::runInEventBaseThread(Func fun)
{
    if(!fun)
    {
        //LOG_ERR
        return false;
    }
    
    if(isRunningEventBase())
    {
        runInLoop(fun);
        return true;
    }
    try
    {
        queue_->putMessage(fun);//队列有长度限制，putMessage可能会抛出异常
    }catch(const std::exception& e)
    {
        //LOG-ERR
        std::cout<<"runInEventBaseThread put message into queue faild"<<e.what()<<std::endl;
        return false;//这里不throw出去，需要手动返回
    }
    
    return true;
}

void EventBase::runInLoop(Func fun)
{
    SpinLockGuard g(spinLock_);
    callbacks_.push_back(fun);
}

void EventBase::initNotificationQueue()
{
    queue_.reset(new NotificationQueue<Func>());
    fnRunner_.reset(new FunctionRunner());
}

size_t EventBase::getNotifyQueueSize() const
{
    queue_->size();
}

void EventBase::loopBody(bool once)
{
    pid_.store(pthread_self(), std::memory_order_release);//release operation
    //std::memory_order_acquire内存模型规则-当前线程内读和写此变量
    //的操作都不会被编译器或者处理器reorder到该load之前
    //其他线程里面对此变量的使用release的写操作，只要发生当前线程都能看到同时更新store后的值
    while(!stop_.load(std::memory_order_acquire))
    {
        startConsumingMsg();
        event_base_loop(base_, EVLOOP_ONCE | EVLOOP_NONBLOCK);
        runCallback();
        if(getNotifyQueueSize() <= 0)
        {
            break;
        }
        if(once)
        {
            break;
        }
    }
    pid_.store(NULL, std::memory_order_release);
    stop_ = false;
}

void EventBase::runCallback()
{
    std::vector<Func> tcallback;
    
    spinLock_.lock();
    tcallback.swap(callbacks_);
    spinLock_.unlock();

    for(auto& itr : tcallback)
    {
        itr();
    }
}

bool EventBase::isRunningEventBase()
{
    int ret = pthread_equal(pid_.load(std::memory_order_acquire), 0);
    
    return ret == 0 ? false : true;
}

bool EventBase::isInEventBaseThread()
{
    int ret = pthread_equal(pthread_self(), pid_.load(std::memory_order_acquire));
    
    return ret == 0 ? false : true;
}

void EventBase::terminateLoop()
{
    stop_.store(true, std::memory_order_release);//release memory barrier
    event_base_loopbreak(base_);
}

}//libext
