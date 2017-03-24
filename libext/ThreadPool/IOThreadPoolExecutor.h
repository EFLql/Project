#pragma once
#include <libext/ThreadPool/ThreadPoolExecutor.h>
#include <libext/asyn/EventBase.h>
#include <libext/EventBaseManger.h>
#include <libext/lock/RWLock.h>
#include <libext/detail/Cache.h>

namespace libext
{
//具体和业务相关的线程池
class IOThreadPoolExecutor : public ThreadPoolExecutor
{
public:
    IOThreadPoolExecutor(int numThreads); 
    ~IOThreadPoolExecutor();
    void addTask(libext::Func fun, libext::Func expireCallback) override;
    EventBase* getEventBase(ThreadPoolExecutor::Thread* h); 
private: 
    struct LIBEXT_ALIGNED_AVOID_FALSE_SHARED IOThread : public Thread
    {
        IOThread(): evb(NULL), shouldRunning(true) {}
        ~IOThread() {} 
        std::atomic<bool> shouldRunning; 
        EventBase* evb;
    };
    void stopThreads(int n) override;
    ThreadPtr pickThread();
    ThreadPtr makeThread() override;
    void threadRun(ThreadPtr thread) override;
private:
    bool isJoin_;
};

} //libext
