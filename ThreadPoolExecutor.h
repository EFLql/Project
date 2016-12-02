/*	@file ThreadPoolExecutor
/*	@brief 线程池执行类
/*
/*	@author liqilin
/*	@date	2016/12/2

/*	@note
/*	@note
/*	
/*	@warning
/*
 */
#include "typedefine.h"
#include "Executor.h"
#include "ThreadFactory.h"

namespace libext
{

class ThreadPoolExecutor : public Executor
{
public:
	ThreadPoolExecutor(int32_t numThreads, ThreadFactoryPtr threadFactory);
	virtual void add(libext::Func func) = 0;
	
	void setThreadFactory(ThreadFactoryPtr threadFactory)
	{
		threadFactory_ = threadFactory;
	}

	ThreadFactoryPtr getThreadFactory()
	{
		return threadFactory_;
	}

	int32_t numThreads();
	void setThreadNums(int32_t numThreads);
	
	void stop();
	void join();

	struct PoolStats 
	{
		PoolStats() : threadCount(0), idleThreadCount(0), activeThreadCount(0),
			pendingTaskCount(0), totalTaskCount(0) {}
		int32_t threadCount, idleThreadCount, activeThreadCount;
		uint64_t pendingTaskCount, totalTaskCount;
	};

	PoolStats getPoolStats();

	struct Task
	{

	};
private:
	ThreadFactoryPtr threadFactory_;
};


}//namespace libext