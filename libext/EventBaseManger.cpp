#include <libext/EventBaseManger.h>

namespace libext
{
EventBaseManager::EventBaseManager()
{
    pthread_key_create(&tkey_, cleanup);
}

EventBaseManager* EventBaseManager::getInstanse()
{
    static EventBaseManager g_obj;
    return &g_obj;
}

EventBase* EventBaseManager::getEventBase()
{
    EventBase* pEnv;
    pEnv = (EventBase*)pthread_getspecific(tkey_);

    if(!pEnv)
    {
        pEnv = new EventBase();
        pthread_setspecific(tkey_, (void*)pEnv);
    }

    return pEnv;
}

//pthread_key清理函数
//每当线程结束时，系统会自动清理键上的数据块
void EventBaseManager::cleanup(void* pEnv)
{
    if(pEnv)
    {
        delete pEnv;
    }
}

} //libext
