#pragma once
#include <libext/ThreadPool/typedefine.h>
#include <libext/ThreadPool/ThreadFactory.h>
#include <libext/ThreadPool/Task.h>
#include <libext/lock/RWLock.h>
#include <libext/detail/Cache.h>
#include <libext/Semaphore.h>
#include <libext/ThreadPool/BlockingQueue.h>
#include <vector>
#include <atomic>
#include <mutex>
#include <queue>

namespace libext
{

struct PoolStats
{
	PoolStats(): threadCount(0), idleCount(0), pendingTask(0) {}

	int threadCount, idleCount, pendingTask;
};

//和业务无关的线程池,不能直接使用
class ThreadPoolExecutor
{
public:
	ThreadPoolExecutor();
	virtual ~ThreadPoolExecutor();

	void setThreads(int n);
	virtual void stopThreads(int n) {}
    int32_t numThreads()
    {
        libext::ReadLockGuard guard(rwLock_);
        return vectThreads_.size();
    }    
	void stopAllThreads();

	PoolStats getPoolStats();

	virtual void addTask(libext::Func fun, libext::Func expireCallback) = 0;
	void runTask(TaskPtr task);
    //lql-note:wangle里面对结构体的定义有对齐设置，需要注意//20170315-由于Thread会统一放到
    //线程池的vector里面，为了防止有不同的Thread结构共享同一块CPU cache line
    //导致在多线程环境下，任意一个Thread变动都会使cache line刷新造成性能下降
    struct LIBEXT_ALIGNED_AVOID_FALSE_SHARED  Thread
    {
        Thread(): pendingTask(0), handler(), 
            id(-1), status(IDLE), sem() {}
        enum Status
        {
            BUSY,
            IDLE 
        };
        std::thread handler;//线程句柄
        int32_t id; 
        std::atomic<size_t> pendingTask;
        std::atomic<Status> status;
        Semaphore sem;
    };
    typedef std::shared_ptr<Thread> ThreadPtr;

	virtual void threadRun(ThreadPtr thread) = 0;

    //Observer interface for thread start/stop
    class Observer
    {
    public:
        virtual void threadStarted(ThreadPtr thread) = 0;
        virtual void threadStoped(ThreadPtr thread) = 0;
        virtual void threadNotYetStoped(ThreadPtr thread) = 0;

        virtual ~Observer() = default;
    };

    virtual void addObserver(std::shared_ptr<Observer> o);
    virtual void removeObserver(std::shared_ptr<Observer> o);
    
    class StopedQueue : public BlockingQueue<ThreadPtr>
    {
    public:
        StopedQueue() {}
        ~StopedQueue() {}
        void add(ThreadPtr item) override;
        ThreadPtr take() override;
        size_t size() override;
        std::mutex mutex_;
        Semaphore sem_;
        std::queue<ThreadPtr> queue_;
    };
protected:
	void addThreads(int n);
	void removeThreads(int n);

protected:
	virtual ThreadPtr makeThread()
	{
		return std::make_shared<Thread>();
	}

	std::vector<ThreadPtr> vectThreads_;
	
	StopedQueue stopQueue_;
    //这里采用读写锁进行保护 
    RWLock rwLock_;  
	ThreadFactory threadFactory_;
    
    std::vector<std::shared_ptr<Observer>> observers_;
};


}//libext
