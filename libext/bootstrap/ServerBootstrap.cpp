#include <libext/bootstrap/ServerBootstrap.h>
#include <libext/Semaphore.h>
namespace libext
{

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

} //libext
