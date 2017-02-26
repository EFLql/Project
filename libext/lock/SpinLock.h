#pragma once
#include <libext/lock/SpinLockImpl.h>

namespace libext
{
typedef SpinLockPthreadImpl SpinLock;

template<typename LOCK>
class SpinLockGuardImpl
{
public:
    SpinLockGuardImpl(LOCK& lock) : lock_(lock)
    {
        lock_.lock();
    }
    ~SpinLockGuardImpl()
    {
        lock_.unlock();
    }
private:
    LOCK& lock_;
};

typedef SpinLockGuardImpl<SpinLock> SpinLockGuard;

} //libext
