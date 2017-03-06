#pragma once
#include <libext/typedef.h>
#include <libext/lock/SpinLock.h>
#include <vector>
#include <event.h>
#include <memory>
namespace libext
{
template<typename MessageT>
class NotificationQueue;
//线程运行主体
class EventBase
{
public:
    EventBase();
    ~EventBase();

    void loop();
    void loopForever();
    //将任务塞到线程队列执行
    bool runInEventBaseThread(Func fun);
    bool isInEventBaseThread();
    void terminateLoop();
    struct event_base* getLibeventBase()
    {
        return base_;
    }
private:
    bool bstop_;
    pid_t pid_;
    void runInLoop(Func fun);
    bool isRunningEventBase();
    void runCallback();
    //线程循环主体
    void loopBody();
    //线程队列
    std::unique_ptr<NotificationQueue<Func>> queue_;
    std::vector<Func> callbacks_;  
    struct event_base* base_;
    
    SpinLock spinLock_;
};


}//libext
