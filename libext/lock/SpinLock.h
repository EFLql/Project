#pragma once

typedef SpinLockPthreadImpl SpinLock;

template<typename LOCK>
class SpinLockGuardImpl
{
public:
    SpinLockGuardImpl(LOCK& lock) : lock_(lock)
    {
        lock_.lock();
    }
    ~SpinLockPthreadImpl()
    {
        lock_.unlock();
    }
private:
    LOCK& lock_;
};

typedef SpinLockGuardImpl<SpinLock> SpinLockGuard;
