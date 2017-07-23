#include <folly/futures/Future.h>
#include <folly/Executor.h>
#include <thread>
#include <iostream>
#include <unistd.h>
#include <pthread.h>

using namespace folly;

int sum()//1+2+3+4+5+6.....+1000
{
    std::cout<<"sum() 任务执行线程id "<<pthread_self()<<std::endl;
    int sum = 0;
    for(int i = 1; i <= 100000000; i ++)
    {
        sum += i;
    }
    return sum;
}

void business1()
{
    std::cout<<"business1 running"<<std::endl;
}

void business2()
{
    std::cout<<"business2 running"<<std::endl;
}

class NewThreadExecutor : public Executor
{
public:
    NewThreadExecutor() {}
    ~NewThreadExecutor() {
        std::for_each(v_.begin(), v_.end(), [](std::thread& t){t.join(); });
    }

    void add(Func f) override {
        v_.emplace_back(std::move(f));
    }

    void addWithPriority(Func f, int8_t /*priority*/) override {
        add(std::move(f));
    }

    uint8_t getNumPriorities() const override {
        return 2;
    }

private:
    std::vector<std::thread> v_;
};

typedef struct Context
{
    Promise<int> promise;
    std::thread t;
}* PContext;

PContext pCtx = NULL;

int sumWithPromise()
{
    pCtx->promise.setValue(sum());
    return 0;
}

Future<int> calculateSum() {
    pCtx = new Context(); 
    Future<int> fut = pCtx->promise.getFuture();
    
    pCtx->t = std::thread(sumWithPromise);
    return std::move(fut);
}

int main()
{
    std::cout<<"主线程id "<<pthread_self()<<std::endl;
    //仍然在主线程里面执行，如果要在不同线程内执行需要使用via()指定执行的线程Executor
    Future<int> future = makeFuture(1).then(sum);
    //logic for business
    business1();
    business2();

    std::cout<<"future isReady?"<<future.isReady()<<std::endl;
    future.wait();
    assert(future.isReady());
    std::cout<<"result of sum is "<<future.get()<<std::endl;
    
    std::cout<<"-----------future2新线程中执行任务-------------------"<<std::endl;
    //新开线程执行任务
    NewThreadExecutor e;
    Future<int> future2 = makeFuture(1).via(&e).then(sum);
    business1();
    business2();
    
    std::cout<<"future2 isReady?"<<future2.isReady()<<std::endl;
    future2.wait();
    assert(future2.isReady());
    std::cout<<"result of sum is "<<future2.get()<<std::endl;
    
    std::cout<<"-----------future3 Promise新线程中执行任务-------------------"<<std::endl;
    Future<int> future3 = std::move(calculateSum());
    business1();
    business2();
    
    std::cout<<"future3 isReady?"<<future3.isReady()<<std::endl;
    future3.wait();
    assert(future3.isReady());
    std::cout<<"result of sum is "<<future3.get()<<std::endl;
    pCtx->t.join();
    delete pCtx;

    return 0;
}
