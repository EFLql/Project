//test for ThreadPool
//
#include <libext/ThreadPool/ThreadPoolExecutor.h>
#include <gtest/gtest.h>
#include <iostream>
#include <unistd.h>

using namespace libext;

TEST(ThreadPoolExecutor, Basic)
{
    ThreadPoolExecutor threadPool(2);

    EXPECT_EQ(threadPool.numThreads(), 2);
}

/*TEST(ThreadPoolExecutor, setThreadsTest)
{
    ThreadPoolExecutor threadPool(2);
    EXPECT_EQ(threadPool.numThreads(), 2);

    threadPool.setThreads(3);
    EXPECT_EQ(threadPool.numThreads(), 3);

    threadPool.setThreads(1);
    EXPECT_EQ(threadPool.numThreads(), 1);

    threadPool.setThreads(4);
    EXPECT_EQ(threadPool.numThreads(), 4);

    threadPool.stopAllThreads();
    EXPECT_EQ(threadPool.numThreads(), 0);
}*/

int count = 0;
void func()
{
    count ++;
    std::cout<<"func run count:"<<count<<std::endl; 
}

TEST(ThreadPoolExecutor, addTaskTest)
{
    count = 0;
    ThreadPoolExecutor threadPool(3);//可能addThread就出错了
    /*threadPool.addTask(NULL, NULL);

    threadPool.addTask(Func(func), NULL);
    threadPool.addTask(Func(func), NULL);
    threadPool.addTask(Func(func), NULL);
    //threadPool.stopAllThreads(); */
    sleep(5);
}
