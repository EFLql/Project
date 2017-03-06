#pragma once
#include <iostream>
#include <semaphore.h>

namespace libext
{
class Semaphore
{
public:
    Semaphore(int count = 0)
    {
        sem_init(&sem_, 0, 0);
    }
    ~Semaphore()
    {
        sem_destroy(&sem_);
    }

    void post()
    {
        sem_post(&sem_);
    }

    void wait()
    {
        sem_wait(&sem_);
    }
    int getSemVal(int* pRet)
    {
        return sem_getvalue(&sem_, pRet);
    }
private:
    sem_t sem_;
};

}//libext
