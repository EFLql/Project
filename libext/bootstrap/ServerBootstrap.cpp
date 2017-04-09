#include <libext/bootstrap/ServerBootstrap.h>
#include <libext/Semaphore.h>
namespace libext
{
ServerBootstrap::ServerBootstrap()
    :reusePort_(false)
{
}

ServerBootstrap::~ServerBootstrap()
{
}

void ServerBootstrap::bind(int port)
{
    libext::SocketAddr addr;
    addr.setFromLocalPort(port);
    bind(addr);
}

void ServerBootstrap::bind(libext::SocketAddr& addr)
{
    if(!workFactory_)
    {
        group(NULL);
    }
    
    bool reusePort = reusePort_ || (accept_group_->numThreads() > 1);
    std::mutex sock_lock;

    Semaphore sem;

    //创建新的socket的过程 
    auto startupSocket = [&]()
    {
        auto socket =socketFactory_->newSocket(addr, 5, reusePort);//create a new obj and bind、listen
        sock_lock.lock();
        sockets_.push_back(socket);
        sock_lock.unlock();

        sem.post();
    };

    //在另外一个线程里面执行创建socket的任务
    accept_group_->addTask(startupSocket, NULL);
    //等待任务执行完成，使用semaphore进行线程同步
    sem.wait();

    //将workfactory_里面的acceptor都取出来注册到新建立的AsyncSocketBase里面
    //注册过程也在accept_group线程池里面完成，主线程不阻塞
    for(auto& socket : sockets_)
    {
        workFactory_->forEachWork([this, socket](Acceptor* work){
               socket->getEventBase()->runInEventBaseThread([this, socket, work]() {
                   socketFactory_->addAcceptCB(socket, work, work->getEventBase());
                   });
               });
    }
}

void ServerBootstrap::group(std::shared_ptr<IOThreadPoolExecutor> io_group)
{
    group(io_group, NULL);
}

void ServerBootstrap::group(std::shared_ptr<IOThreadPoolExecutor> io_group,
                            std::shared_ptr<IOThreadPoolExecutor> accept_group)
{
    if(!io_group)
    {
        int threads = std::thread::hardware_concurrency();
        if(threads <= 0)
        {
            threads = 8;
        }
        io_group = std::make_shared<libext::IOThreadPoolExecutor>(threads);
    }
    if(!accept_group)
    {
        accept_group = std::make_shared<libext::IOThreadPoolExecutor>(1);
    }
    if(!acceptorFactory_)
    {
        acceptorFactory_ = 
            std::make_shared<ServerAcceptorFactory<DefaultPipeline>>(childPipelineFactory_);
    }
    workFactory_ = std::make_shared<ServerWorkerPool>(
            acceptorFactory_, socketFactory_, sockets_, io_group.get());
    io_group->addObserver(workFactory_);
    io_group_ = io_group;
    accept_group_ = accept_group;
}


//////////////////////////////////////////////////////////
ServerWorkerPool::~ServerWorkerPool()
{
}

void ServerWorkerPool::threadStarted(ThreadPoolExecutor::ThreadPtr thread)
{
    auto worker = acceptorFactory_->newAcceptor(exec_->getEventBase(thread.get()) );
    {
        libext::SpinLockGuard g(spinLock_);
        worker_->insert(make_pair(thread, worker));
    }
    
    //将新生成的worker对象注册到所有的AsyncSocketBase对象里面
    for(auto& socket : sockets_)
    {
        socket->getEventBase()->runInEventBaseThread([this,socket, worker](){
                socketFactory_->addAcceptCB(socket, worker.get(), worker->getEventBase());
                });
    }
}

//线程停止
void ServerWorkerPool::threadStoped(ThreadPoolExecutor::ThreadPtr thread)
{
     
}

template<typename F>
void ServerWorkerPool::forEachWork(F&& f)
{
    libext::SpinLockGuard g(spinLock_);
    for(auto& woker : *worker_)
    {
        f(woker.second.get()); 
    }
}


} //libext
