#pragma once
#include <libext/asyn/EventBase.h>
#include <pthread.h>

namespace libext
{
//返回当前线程EventBase私有数据
class EventBaseManager
{
public:
    EventBaseManager(const EventBaseManager& that) = delete; 
    EventBaseManager(const EventBaseManager&& that) = delete;
    
    //获取全局实例对象
    static EventBaseManager* getInstanse();
    EventBase* getEventBase();
private:
    //禁止用户直接实例化对象
    EventBaseManager();
    static void cleanup(void* pEnv);

private:
    pthread_key_t tkey_;
};

}//libext
