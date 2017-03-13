#pragma once
#include <libext/ThreadPool/ThreadPoolExecutor.h>
#include <libext/asyn/EventBase.h>
#include <libext/EventBaseManger.h>

namespace libext
{
//具体和业务相关的线程池
class IOThreadPoolExecutor : public ThreadPoolExecutor
{
public:
    IOThreadPoolExecutor(int numThreads); 
    ~IOThreadPoolExecutor();
    void addTask(libext::Func fun, libext::Func expireCallback) override;
    libext::ThreadPtr pickThread();

    void threadRun(ThreadPtr thread) override;
    void addObserver(std::shared_ptr<libext::ThreadPoolExecutor::Observer> o) override; 
    void removeObserver(std::shared_ptr<libext::ThreadPoolExecutor::Observer> o) override;
    
    struct IOThread : public Thread
    {
        IOThread();

        EventBase* evb;
    };
};

} //libext
