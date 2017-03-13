#include <iostream>
#include "ThreadPoolExecutor.h"
#include <unistd.h>

namespace libext
{
ThreadPoolExecutor::ThreadPoolExecutor(int numthreads)
{
    setThreads(numthreads);
}


ThreadPoolExecutor::~ThreadPoolExecutor()
{
}

//add thread manager
void ThreadPoolExecutor::addThreads(int n)
{
	//std::lock_guard<std::mutex> guard(mutexvectThreads_);
	
	for(int i = 0; i < n; i ++)
	{
        std::cout<<"add "<<i<<" thread"<<std::endl;
		auto thread = makeThread();
        thread->shouldRunning.store(true, std::memory_order_release);   
		thread->handler_ = threadFactory_.newThread( std::bind(&ThreadPoolExecutor::threadRun, this, thread) );
		vectThreads_.push_back(std::move(thread) );
	}
}


void ThreadPoolExecutor::removeThreads(int n)
{
    //std::lock_guard<std::mutex> guard(mutexvectThreads_);
    std::vector<ThreadPtr>::iterator itr;
    stopThreads(n);

    for(int i = 0; i < n;i ++)
    {
        auto thread = stopQueue_.take(true);
        thread->handler_.join();
        for(itr = vectThreads_.begin(); itr != vectThreads_.end(); )
        {
            if(*itr == thread)
            {
                itr = vectThreads_.erase(itr);
                break;
            }
            else
            {
                itr++;
            }
        }
        std::cout<<"remove "<<i<<" thread"<<std::endl;
    }    
}

void ThreadPoolExecutor::setThreads(int n)
{
    std::lock_guard<std::mutex> guard(mutexvectThreads_);
    int current = vectThreads_.size();
    if(current < n)
    {
        addThreads(n - current);
    }
    else
    {
        removeThreads(current - n);
    }
}

void ThreadPoolExecutor::threadRun(ThreadPtr thread)
{
    while(thread->shouldRunning.load(std::memory_order_acquire))
    {
        //runing the thread
        Thread::TaskWrapper taskwrapper = thread->getTask();
        if(NULL == taskwrapper) usleep(10);
        else taskwrapper(NULL); 
    }

    //add it to queue
    stopQueue_.pushback(thread);
}

void ThreadPoolExecutor::stopThreads(int n)
{
    std::vector<ThreadPtr>::iterator itr = vectThreads_.begin();
    for(int i = 0; i < n, itr != vectThreads_.end(); i++, itr++)
    {
        auto thread = *itr;
        thread->shouldRunning.store(false, std::memory_order_release);
    }
}

void ThreadPoolExecutor::stopAllThreads()
{
    std::lock_guard<std::mutex> guard(mutexvectThreads_);
    int n = vectThreads_.size();
    removeThreads(n);
}

PoolStats ThreadPoolExecutor::getPoolStats()
{
    std::lock_guard<std::mutex> guard(mutexvectThreads_);
    PoolStats stats;
    stats.threadCount = vectThreads_.size();
    for(int i =0; i < stats.threadCount; i ++)
    {
        if(vectThreads_[i]->idle.load(std::memory_order_acquire)) stats.idleCount ++;

    }
    return stats;
}


//add schedule
/*void ThreadPoolExecutor::addTask(libext::Func fun, libext::Func expireCallback)
{
    auto thread = pickThread();
    TaskPtr task = std::make_shared<Task>(fun, expireCallback, std::chrono::milliseconds(100) );
    Thread::TaskWrapper taskwrapper = std::bind(&ThreadPoolExecutor::runTask, this, std::move(task) ); 
    thread->addTask(std::move(taskwrapper) );
}*/

void ThreadPoolExecutor::runTask(TaskPtr task)
{
    std::cout<<"run task"<<std::endl;
    std::chrono::steady_clock::time_point runtime = std::chrono::steady_clock::now();
    if(runtime - task->pendingTime_ > task->expireTime_)//task is expired
    {
        if(task->expireCallback_) task->expireCallback_();
    }
    else
    {
        task->runTime_ = runtime;
        try
        {
            task->func_();
        }catch(const std::exception& e)
        {
            std::cout<<"ThreadPoolExecutor threw a unhandler exception "<<e.what()<<std::endl;
        }
        
    }
}

/*libext::ThreadPtr ThreadPoolExecutor::pickThread()
{
    static int s = 0;
    s = (s + 1) % vectThreads_.size();
    return vectThreads_[s];
}*/


} //libext
