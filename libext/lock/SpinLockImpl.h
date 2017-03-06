//自旋锁封装
#pragma once
#include <pthread.h>

namespace libext
{
class SpinLockPthreadImpl
{
public:
    SpinLockPthreadImpl()
    {
        pthread_spin_init(&lock_, PTHREAD_PROCESS_PRIVATE);
    }
    ~SpinLockPthreadImpl()
    {
        pthread_spin_destroy(&lock_);
    }

    inline void lock() 
    {
        pthread_spin_lock(&lock_); 
    }
    inline bool trylock() 
    {
        int rc = pthread_spin_trylock(&lock_);
        if(rc == 0)
        {
            return true;
        }
        return false; 
    }
    inline void unlock() 
    {
        pthread_spin_unlock(&lock_); 
    }

private:
    pthread_spinlock_t lock_;
};

} //libext
