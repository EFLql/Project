#pragma once
#include <libext/ThreadPool/ThreadPoolExecutor.h>
#include <libext/asyn/EventBase.h>
#include <libext/EventBaseManger.h>

namespace libext
{

class IOThreadPoolExecutor : public ThreadPoolExecutor
{
public:
    IOThreadPoolExecutor(int numThreads); 
    ~IOThreadPoolExecutor();

    EventBase* getEventBase() const
    {
        return libext::EventBaseManager::getInstanse()->getEventBase();
    }
    void threadRun(ThreadPtr thread) override;
    void addObserver(std::shared_ptr<libext::ThreadPoolExecutor::Observer> o) override; 
    void removeObserver(std::shared_ptr<libext::ThreadPoolExecutor::Observer> o) override;
};

} //libext
