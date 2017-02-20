#include "EventBaseManger.h"

using libext::EventBaseManager;

EventBaseManager::EventBaseManger()
{
    pthread_key_create(tkey_, cleanup);
}

EventBaseManager* EventBaseManager::getInstanse()
{
    static EventBaseManager g_obj;
    return &g_obj;
}

EventBase* EventBaseManager::getEventBase()
{
    EventBase* pEnv;
    pthread_getspecific(tkey_, (void*)pEnv);

    if(!pEnv)
    {
        pEnv = new EventBase();
        pthread_setspecific(tkey_, (void*)pEnv);
    }

    retur pEnv;
}

//pthread_key清理函数
void EventBaseManager::cleanup(void* pEnv)
{
    if(pEnv)
    {
        delete p;
    }
}
