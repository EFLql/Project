#pragma once
//linux读写锁封装
//特别注意，只有当共享资源在读操作远远大于写操作次数才能
//使用读写锁，否则读写锁的性能会比普通的mutex低很多
//在使用之前需要分析清除

#include <pthread.h>

namespace libext
{
class RWLockImpl
{
public:
    RWLockImpl()
    {
        pthread_rwlock_init(&rwLock_, NULL);
    }
    ~RWLockImpl()
    {
        pthread_rwlock_destroy(&rwLock_);
    }

    int rdLock()//获取读锁
    {
        return pthread_rwlock_rdlock(&rwLock_);
    }
    int wrLock()//获取写锁
    {
        return pthread_rwlock_wrlock(&rwLock_);
    }
    
    void unlock()
    {
        pthread_rwlock_unlock(&rwLock_);
    }
    
    //非阻塞的读写锁
    int tryrdLock()
    {
        return pthread_rwlock_tryrdlock(&rwLock_);
    }
    int trywrLock()
    {
        return pthread_rwlock_trywrlock(&rwLock_);
    }
private:
    pthread_rwlock_t rwLock_;
};

} //libext
