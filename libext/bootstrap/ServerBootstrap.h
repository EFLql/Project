#pragma once
#include <libext/bootstrap/acceptor/Acceptor.h>
#include <libext/ThreadPool/IOThreadPoolExecutor.h>
#include <libext/bootstrap/ServerSocketFactory.h>
#include <map>
#include <memory>
namespace libext
{
class ServerAcceptor : public Acceptor
{
};


class ServerAcceptorFactory : public AcceptorFactory
{
};

class ServerWorkerPool : public ThreadPoolExecutor::Observer
{
public:
    ServerWorkerPool(const ServerWorkerPool& that) = delete;
    ServerWorkerPool(std::shared_ptr<AcceptorFactory> acceptorFactory, 
                    std::shared_ptr<ServerSocketFactory> socketFactory,
                    std::vector<std::shared_ptr<AsyncSocketBase>> s,
                    IOThreadPoolExecutor* exe)
        : acceptorFactory_(acceptorFactory)
        , socketFactory_(socketFactory)
        , exec_(exe)
        , sockets_(s)
        , worker_(std::make_shared<WorkerMap>()){}
    ~ServerWorkerPool(); 

    void threadStarted(ThreadPtr thread);
    void threadStoped(ThreadPtr thread);
    void threadNotYetStoped(ThreadPtr thread)
    {
        threadStoped(thread);
    }
    template<typename F>
    void forEachWork(F&& f);

private:
    //c++11 using could replace typedef
    //using WorkerMap = std::map<ThreadPtr, std::shared_ptr<Acceptor>>;
    typedef std::map<ThreadPtr, std::shared_ptr<Acceptor>> WorkerMap;
    std::shared_ptr<WorkerMap> worker_;
    std::shared_ptr<AcceptorFactory> acceptorFactory_;
    std::shared_ptr<ServerSocketFactory> socketFactory_;
    IOThreadPoolExecutor* exec_; 
    std::vector<std::shared_ptr<AsyncSocketBase>> sockets_;

};

class ServerBootstrap
{
public:
    ServerBootstrap();
    ~ServerBootstrap();

    void bind(int port);
    void bind(libext::SocketAddr& addr);
    void group(std::shared_ptr<IOThreadPoolExecutor> io_group);

    void group(std::shared_ptr<IOThreadPoolExecutor> io_group,
            std::shared_ptr<IOThreadPoolExecutor> accept_group);

private:
    std::vector<std::shared_ptr<AsyncSocketBase>> sockets_;
    std::shared_ptr<ServerSocketFactory> socketFactory_;
    std::shared_ptr<ServerAcceptorFactory> acceptorFactory_;
    std::shared_ptr<ServerWorkerPool> workFactory_;
    std::shared_ptr<libext::IOThreadPoolExecutor> io_group_;
    std::shared_ptr<libext::IOThreadPoolExecutor> accept_group_;
    bool reusePort_;
};


}//libext
