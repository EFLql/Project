#pragma once
#include <libext/typedef.h>
#include <libext/lock/SpinLock.h>
#include <vector>
#include <event.h>
#include <memory>
#include <atomic>

namespace libext
{

template<typename MessageT>
class NotificationQueue;
class FunctionRunner;//forward declare

//线程运行主体
class EventBase
{
public:
    EventBase();
    ~EventBase();

    void loop();
    void loopForever();
    void loopOnce();
    //将任务塞到线程队列执行
    bool runInEventBaseThread(Func fun);
    bool isInEventBaseThread();
    void terminateLoop();
    struct event_base* getLibeventBase()
    {
        return base_;
    }
    size_t getNotifyQueueSize() const;
private:
    void initNotificationQueue();
    void runInLoop(Func fun);
    bool isRunningEventBase();
    void runCallback();
    //线程循环主体
    void loopBody(bool once = false);
    //线程队列
    std::unique_ptr<NotificationQueue<Func>> queue_;
    std::unique_ptr<FunctionRunner> fnRunner_;
    void startConsumingMsg();
    std::vector<Func> callbacks_;  
    struct event_base* base_;
    
    SpinLock spinLock_;
    std::atomic<bool> stop_;
    std::atomic<pthread_t> pid_;

};


}//libext
