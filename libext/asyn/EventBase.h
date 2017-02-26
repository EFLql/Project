#pragma once
#include <libext/asyn/NotificationQue.h>
#include <libext/lock/SpinLock.h>
#include <vector>

namespace libext
{
//线程运行主体
class EventBase
{
public:
    EventBase();
    ~EventBase();

    void loop();
    void loopForever();
    //将任务塞到线程队列执行
    void runInEventBaseThread(Func fun);
    bool isInEventBaseThread();
private:
    bool bstop_;
    pid_t pid_;
    void runInLoop(Func fun);
    bool isRuningInEventBase();
    void runCallback();
    //线程循环主体
    void loopBody();
    //线程队列
    NotificationQueue<Func> queue_;
    std::vector<Func> callbacks_;  
    struct event_base* base_;
    
    SpinLock spinLock_;
};


}//libext
