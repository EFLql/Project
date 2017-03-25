#include <iostream>
#include "ThreadPoolExecutor.h"
#include <unistd.h>

namespace libext
{
ThreadPoolExecutor::ThreadPoolExecutor()
{
}


ThreadPoolExecutor::~ThreadPoolExecutor()
{
}

//add thread manager
void ThreadPoolExecutor::addThreads(int n)
{
    std::cout<<"add "<<" thread"<<std::endl;
    std::vector<ThreadPtr> newThreads;
    for(int i = 0; i < n; i ++)
    {
        auto thread = makeThread();
	    thread->id = i;	
        newThreads.push_back(thread);
    }
	for(auto& thread : newThreads)
	{
		thread->handler = threadFactory_.newThread(
                std::bind(&ThreadPoolExecutor::threadRun, this, thread));
        vectThreads_.push_back(thread);
	}
    for(auto& thread : newThreads)
    {
        thread->sem.wait();//需要等EentBase启动好之后才能通知所有的observer
    }
    for(auto& o : observers_)
    {
        for(auto& thread : newThreads)
        {
            o->threadStarted(thread);
        }
    }
}


void ThreadPoolExecutor::removeThreads(int n)
{
    std::vector<ThreadPtr>::iterator itr;
    stopThreads(n);

    for(int i = 0; i < n;i ++)
    {
        auto thread = stopQueue_.take();
        thread->handler.join();
        for(itr = vectThreads_.begin(); itr != vectThreads_.end(); )
        {
            if(itr->get()->id == thread->id)
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
    libext::WriteLockGuard guard(rwLock_);
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

void ThreadPoolExecutor::stopAllThreads()
{
    libext::WriteLockGuard guard(rwLock_);
    int n = vectThreads_.size();
    removeThreads(n);
}

PoolStats ThreadPoolExecutor::getPoolStats()
{
    libext::ReadLockGuard guard(rwLock_);
    PoolStats stats;
    stats.threadCount = vectThreads_.size();
    for(int i =0; i < stats.threadCount; i ++)
    {
        if(vectThreads_[i]->status == Thread::IDLE) stats.idleCount ++;

    }
    return stats;
}

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

//observers_的操作不应该在多线程内操作
void ThreadPoolExecutor::addObserver(std::shared_ptr<libext::ThreadPoolExecutor::Observer> o)
{
    libext::ReadLockGuard g(rwLock_);
    observers_.push_back(o);
    for(auto& thread : vectThreads_)
    {
        o->threadStarted(thread);
    }
}

void ThreadPoolExecutor::removeObserver(std::shared_ptr<libext::ThreadPoolExecutor::Observer> o)
{
    std::vector<std::shared_ptr<Observer>>::iterator itr = observers_.begin();
    for(; itr != observers_.end(); itr ++)
    {
        if(*itr == o)
        {
            observers_.erase(itr);
        }
    }
}

size_t ThreadPoolExecutor::StopedQueue::size()
{
    std::lock_guard<std::mutex> guard(mutex_);
    return queue_.size();
}

void ThreadPoolExecutor::StopedQueue::add(ThreadPtr thread)
{
    std::lock_guard<std::mutex> guard(mutex_);
    queue_.push(thread);
    sem_.post();
}

ThreadPoolExecutor::ThreadPtr ThreadPoolExecutor::StopedQueue::take()
{
    while(true)
    {
        {
            std::lock_guard<std::mutex> guard(mutex_);
            if(!queue_.empty())
            {
                ThreadPtr thread = queue_.front();
                queue_.pop();
                return thread;
            }
        }
        sem_.wait(); 
    }
}

} //libext
