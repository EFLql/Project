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


class ThreadPoolExecutor
{
public:
	ThreadPoolExecutor(int numthreads);
	~ThreadPoolExecutor();

	void setThreads(int n);
	void stopThreads(int n);
	
	void stopAllThreads();

	PoolStats getPoolStats();

	void addTask(libext::Func fun, libext::Func expireCallback);
	void runTask(libext::TaskPtr task);

	void threadRun(libext::ThreadPtr thread);

    ThreadPtr pickThread();

    //Observer interface for thread start/stop
    class Observer
    {
    public:
        virtual void threadStarted(ThreadPtr thread) = 0;
        virtual void threadStoped(ThreadPtr thread) = 0;
        virtual void threadNotYetStoped(ThreadPtr thread) = 0;

        virtual ~Observer() = default;
    };

    void addObserver(std::shared_ptr<Observer> o);
    void removeObserver(std::shared_ptr<Observer> o);

private:
	void addThreads(int n);
	void removeThreads(int n);

protected:
	ThreadPtr makeThread()
	{
		return std::make_shared<Thread>();
	}

private:
	std::vector<libext::ThreadPtr> vectThreads_;
	
	Queue<libext::ThreadPtr> stopQueue_;

	std::mutex mutexvectThreads_;

	ThreadFactory threadFactory_;
    
    std::shared_ptr<Observer> observer_;
};


}//libext
