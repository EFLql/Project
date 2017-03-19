#pragma once
#include <libext/lock/RWLockImpl.h>

namespace libext
{

typedef RWLockImpl RWLock;

template <class LOCK>
class ReadLockGuardImpl
{
public:
    ReadLockGuardImpl(LOCK& lock) : lock_(lock)
    {
        lock_.rdLock();
    }
    ~ReadLockGuardImpl()
    {
        lock_.unlock();
    }
    
private:
    LOCK& lock_;
};

template <class LOCK>
class WriteLockGuardImpl
{
public:
    WriteLockGuardImpl(LOCK& lock) : lock_(lock)
    {
        lock_.wrLock();
    }
    ~WriteLockGuardImpl()
    {
        lock_.unlock();
    }
    
private:
    LOCK& lock_;
};
typedef ReadLockGuardImpl<RWLock> ReadLockGuard;
typedef WriteLockGuardImpl<RWLock> WriteLockGuard;
};
