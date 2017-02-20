//自旋锁封装
#pragma once
#include <pthread.h>

class SpinLockPthreadImpl
{
public:
    SpinLockPthreadImpl()
    {
        if(0 == pthread_spin_init(&lock_)
        {
            return true;
        }
        return false;
    }
    ~SpinLockPthreadImpl()
    {
        if(0 == pthread_spin_destroy(&lock_))
        {
            return true;
        }
        return false;
    }

    inline void lock() const
    {
        if(0 == pthread_spin_lock(&lock_) )
        {
            return true;
        }
        return false;
    }
    inline bool trylock() const
    {
        int rc = pthread_spin_trylock(&lock_);
        if(rc == 0)
        {
            return true;
        }
        return false; 
    }
    inline void unlock() const
    {
        if(0 == pthread_spin_unlock(&lock_) )
        {
            return true;
        }
        return false;
    }

private:
    pthread_spinlock_t lock_;
};
