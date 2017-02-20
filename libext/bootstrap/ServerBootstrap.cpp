#include "ServerBootstrap.h"

using libext::ServerBootstrap;
using libext::ServerWorkerPool;

ServerBootstrap::ServerBootstrap()
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
        //lql-need modify
        auto socket = SocketFactory_->newSocket(addr, reusePort);//create a new obj and bind、listen
        sock_lock.lock();
        sockets_.push(socket);
        sock_lock.unlock();

        sem.post();
    }

    //在另外一个线程里面执行创建socket的任务
    accept_group.addTask(startupSocket);
    //等待任务执行完成，使用semaphore进行线程同步
    sem.wait();

    //将workfactory_里面的acceptor都取出来注册到新建立的AsyncSocketBase里面
    //注册过程也在accept_group线程池里面完成，主线程不阻塞
    for(auto& socket : sockets)
    {
        //lql-need modify
        workFactory_->forEachWork([socket](Acceptor* work){
               socket->getEventBase()->run([socket]() {
                   socketFactory_->addAcceptCB(socket, work, work->getEventBase())
                   })
               });
    }
}

void ServerBootstrap::group(std::shared_ptr<libext::ThreadPoolExecutor> io_group)
{
    group(io_group, NULL);
}

void ServerBootstrap::group(std::shared_ptr<libext::ThreadPoolExecutor> io_group,
                            std::shared_ptr<libext::ThreadPoolExecutor> accept_group)
{
    if(!io_group)
    {
        int threads = std::thread::hardware_concurrency();
        if(threads <= 0)
        {
            threads = 8;
        }
        io_group = std::make_shared<libext::ThreadPoolExecutor>(threads);
    }
    if(!accept_group)
    {
        accept_group = std::make_shared<libext::ThreadPoolExecutor>(1);
    }
    if(!acceptorFactory_)
    {
        acceptorFactory_ = std::make_shared<AcceptorFactory>();
    }
    workFactory_ = std::make_shared<ServerWorkerPool>(acceptorFactory_, ServerSocketFactory_, io_group);
    io_group.addObserver(workFactory_);
    
    io_group_ = io_group;
    accept_group_ = accept_group;
}


//////////////////////////////////////////////////////////
void ServerWorkerPool::threadStarted(ThreadPtr thread)
{
    //lql-need modify
    worker = acceptorFactory_->newAcceptor(exec_->getEventBase() );
    worker_->insert(make_pair(thread, acceptor));
    
    //将新生成的worker对象注册到所有的AsyncSocketBase对象里面
    for(auto& socket : sockets_)
    {
        //lql-need modify
        socket->getEventBase()->run([](){
                socketFactory_->addAcceptCB(socket, worker_, work->getEventBase())
                });
    }
}

//线程停止
void ServerWorkerPool::threadStoped(ThreadPtr thread)
{
     
}
