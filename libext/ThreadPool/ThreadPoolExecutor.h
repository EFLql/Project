#pragma once
#include <vector>
#include "typedefine.h"
#include "Thread.h"
#include "Task.h"
#include "ThreadFactory.h"
#include "Queue.h"

namespace libext
{

struct PoolStats
{
	PoolStats() {}

	int threadCount, idleCount, pendingTask;
};

//和业务无关的线程池,不能直接使用
class ThreadPoolExecutor
{
public:
	ThreadPoolExecutor(int numthreads);
	virtual ~ThreadPoolExecutor();

	void setThreads(int n);
	void stopThreads(int n);
    int32_t numThreads() const
    {
        return vectThreads_.size();
    }    
	void stopAllThreads();

	PoolStats getPoolStats();

	void addTask(libext::Func fun, libext::Func expireCallback) = 0;
	void runTask(libext::TaskPtr task);
    
	virtual void threadRun(libext::ThreadPtr thread) = 0;

    //Observer interface for thread start/stop
    class Observer
    {
    public:
        virtual void threadStarted(ThreadPtr thread) = 0;
        virtual void threadStoped(ThreadPtr thread) = 0;
        virtual void threadNotYetStoped(ThreadPtr thread) = 0;

        virtual ~Observer() = default;
    };

    virtual void addObserver(std::shared_ptr<Observer> o) = 0;
    virtual void removeObserver(std::shared_ptr<Observer> o) = 0;

private:
	void addThreads(int n);
	void removeThreads(int n);

protected:
	ThreadPtr makeThread()
	{
		return std::make_shared<Thread>();
	}

	std::vector<libext::ThreadPtr> vectThreads_;
	
	Queue<libext::ThreadPtr> stopQueue_;

	std::mutex mutexvectThreads_;

	ThreadFactory threadFactory_;
    
    std::shared_ptr<Observer> observer_;
};


}//libext
