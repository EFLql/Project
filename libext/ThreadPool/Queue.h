#pragma once
#include <queue>
#include <mutex>
#include <unistd.h>

namespace libext
{

template<class T>
class Queue
{
public:
    Queue() {}
    virtual ~Queue() {}

    //void pushback(T t)
    //T take();

    
    void pushback(T t)
    {
        std::lock_guard<std::mutex> guard(mutex_);
        queue_.push( std::move(t) );
    }

    T take(bool block = false)//blocking the thread
    {
        while(1)
        {
            {
                std::lock_guard<std::mutex> guard(mutex_);
                if(block == false)
                {
                    if(queue_.empty()) return NULL;
                }

                if(!queue_.empty())
                {
                    T rtn = queue_.front();
                    queue_.pop();
                    return std::move(rtn);
                }
            }
            usleep(100);
        }
    }

private:
    std::mutex mutex_;
    std::queue<T> queue_;
};


}//libext
