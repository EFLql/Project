//test for ThreadPool
//
#include <libext/ThreadPool/IOThreadPoolExecutor.h>
#include <gtest/gtest.h>
#include <iostream>
#include <unistd.h>

using namespace libext;

TEST(ThreadPoolExecutor, Basic)
{
    IOThreadPoolExecutor threadPool(2);

    EXPECT_EQ(threadPool.numThreads(), 2);

    threadPool.stopAllThreads();
}

TEST(ThreadPoolExecutor, setThreadsTest)
{
    IOThreadPoolExecutor threadPool(2);
    EXPECT_EQ(threadPool.numThreads(), 2);

    threadPool.setThreads(3);
    EXPECT_EQ(threadPool.numThreads(), 3);

    threadPool.setThreads(1);
    EXPECT_EQ(threadPool.numThreads(), 1);

    threadPool.setThreads(4);
    EXPECT_EQ(threadPool.numThreads(), 4);

    threadPool.stopAllThreads();
    EXPECT_EQ(threadPool.numThreads(), 0);
}

int count = 0;
void func()
{
    count ++;
    std::cout<<"func run count:"<<count<<std::endl; 
}

TEST(ThreadPoolExecutor, addTaskTest)
{
    count = 0;
    IOThreadPoolExecutor threadPool(3);//可能addThread就出错了
    threadPool.addTask(NULL, NULL);

    threadPool.addTask(Func(func), NULL);
    threadPool.addTask(Func(func), NULL);
    threadPool.addTask(Func(func), NULL);
    threadPool.stopAllThreads(); 
    EXPECT_EQ(count, 3);
}

void sleepTest()
{
    sleep(5);//休眠5秒
}
TEST(ThreadPoolExecutor, sleepTest)
{
    IOThreadPoolExecutor threadPool(3);
    threadPool.addTask(Func(sleepTest), NULL);
    threadPool.stopAllThreads(); 
}

void expireFunc(bool* prove)
{
    if(prove) *prove = true;
    std::cout<<"expireFunc is Running"<<std::endl;
}

TEST(ThreadPoolExecutor, expireTest)
{
    IOThreadPoolExecutor threadPool(1);
    bool prove = false;
    threadPool.addTask(Func(sleepTest), NULL);
    threadPool.addTask(NULL, std::bind(expireFunc, &prove));//bind传递的参数是以值传递的
    threadPool.stopAllThreads(); 
    EXPECT_EQ(prove, true);
}

void getThreadPoolStatus(IOThreadPoolExecutor* threadPool)
{
    while(true)
    {
        if(threadPool->numThreads() == 0) break;
        libext::PoolStats status = threadPool->getPoolStats();
        std::cout<<"Poolstaus: threads "<<status.threadCount\
            <<"idle nums: "<<status.idleCount<<std::endl;
    }
}
TEST(ThreadPoolExecutor, threadPoolStatusTest)
{
    IOThreadPoolExecutor threadPool(4);
    std::thread t(getThreadPoolStatus, &threadPool);
    bool prove = false;
    for(int i = 0; i < 50000; i ++)//5w
    {
        threadPool.addTask([i](){
                    std::cout<<"the "<<i<<" task run!"<<std::endl;
                }, std::bind(expireFunc, &prove));
    }

    threadPool.stopAllThreads();
    t.join();
}
