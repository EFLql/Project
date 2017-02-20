#pragma once

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
    //线程循环主体
    void loopBody();
    bool isInEventBaseThread();
private:
    bool bstop_;
    pid_t pid_;
    //lql-need modify
    //线程队列
    NotificationQueue<Func> queue_;
    
    struct event_base* base_;
};


}//libext
