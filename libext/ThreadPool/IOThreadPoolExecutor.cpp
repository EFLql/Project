#include <libext/ThreadPool/IOThreadPoolExecutor.h>
#include <assert.h>

namespace libext
{
IOThreadPoolExecutor::IOThreadPoolExecutor(int numThreads)
: isJoin_(true)
{
   addThreads(numThreads); 
}

IOThreadPoolExecutor::~IOThreadPoolExecutor()
{
}

void IOThreadPoolExecutor::addTask(libext::Func fun, libext::Func expireCallback)
{
    auto thread = std::static_pointer_cast<IOThread>(pickThread());
    assert(thread->evb);
    TaskPtr task = std::make_shared<Task>(fun, expireCallback,std::chrono::milliseconds(100));
    task->pendingTime_ = std::chrono::steady_clock::now();
    auto funcWrapper = [this, task, thread]() {
       thread->status = Thread::BUSY;
       runTask(std::move(task));
       thread->pendingTask --; 
    };
    thread->pendingTask ++;
    if(!thread->evb->runInEventBaseThread(std::move(funcWrapper)))
    {
        thread->pendingTask --;
        throw std::runtime_error("Unable run fun in eventbase thread");
    }
}

//schedule thread
ThreadPoolExecutor::ThreadPtr IOThreadPoolExecutor::pickThread()
{
    //需要加读写锁
    libext::ReadLockGuard guard(rwLock_);
    static int s = 0;
    s = (s + 1) % vectThreads_.size();
    return vectThreads_[s];
}

ThreadPoolExecutor::ThreadPtr IOThreadPoolExecutor::makeThread()
{
    return std::make_shared<IOThread>();
}

void IOThreadPoolExecutor::threadRun(ThreadPtr thread)
{
    std::cout<<"thread Run"<<thread.get()<<std::endl;
    std::shared_ptr<IOThread> ioThread = std::static_pointer_cast<IOThread>(thread);
    ioThread->evb = EventBaseManager::getInstanse()->getEventBase();
    
    ioThread->evb->runInEventBaseThread([ioThread] () {
            ioThread->sem.post(); });//必须确保EventBase循环启动了才通知所有的observer
    while(ioThread->shouldRunning)//shouldrunning是atomic类型的变量，这里使用默认的内存顺序
    {
        ioThread->evb->loopForever();
    }
    if(isJoin_)
    { 
        while(ioThread->pendingTask > 0)
        {
            ioThread->evb->loopOnce();
        }
    }
    //stop EventBase's loop
    ioThread->evb->terminateLoop();
    //evb对象在线程结束时由系统自动调用cleanup函数清理 
    //所以无须手动调用EentManager的cleanup函数清理 
    //lql-note 与wangle不同之处
    stopQueue_.add(ioThread);
}



EventBase* IOThreadPoolExecutor::getEventBase(ThreadPoolExecutor::Thread* h)
{
    auto ioThread = static_cast<IOThread*>(h);
    if(ioThread)
    {
        return ioThread->evb;
    }
    return NULL;
}

void IOThreadPoolExecutor::stopThreads(int n)
{
    std::cout<<"stopThread "<<n<<std::endl;
    std::vector<ThreadPtr>::iterator itr = vectThreads_.begin();
    for(; itr != vectThreads_.end(); itr++)
    {
        auto ioThread = std::static_pointer_cast<IOThread>(*itr);
        ioThread->shouldRunning = false;
    }
}
} //libext
