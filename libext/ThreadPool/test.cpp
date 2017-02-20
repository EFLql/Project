#include <iostream>
#include <unistd.h>
#include <string.h>
#include "typedefine.h"
#include "ThreadPoolExecutor.h"

int test[3] = {0,3,5};

int main(int argc, char* argv[])
{
    for(int j = 1; j < argc; j ++)
    {
        test[j] = 0;
        for(int i = 0; i < strlen(argv[j]); i ++)
        {
            test[j] = test[j] * 10 + argv[j][i] - '0';
        }
    }
    libext::ThreadPoolExecutor pool;
    pool.setThreads(test[1]);

    libext::PoolStats stats = pool.getPoolStats();
    
    int count = 0;
    auto func1 = [&] { std::cout<<"task running count= "<<count<<std::endl;  count ++; };
    auto expireCallback = [] (int itask) { std::cout<<"The "<<itask<<" task is expire"<<std::endl; };

    for(int i = 0; i < test[2]; i ++)
    {
        auto expireWrapper = std::bind(expireCallback, i);    
        pool.addTask(func1, expireWrapper);
    }
    
    std::cout<<"add task over"<<std::endl;

    sleep(2);
    pool.stopAllThreads();
}
