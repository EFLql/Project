#pragma once

namespace libext
{
class ServerAcceptor : public Acceptor
{
};


class ServerAcceptorFactory : public AcceptorFactory
{
};

class ServerWorkerPool : public ThreadPoolExecutor::Oberver
{
public:
    ServerWorkerPool(const ServerWorkerPool& that) = delete;
    ServerWorkerPool(std::shared_ptr<AcceptorFactory> acceptorFactory, 
                    std::shared_ptr<ServerSocketFactory> socketFactory
                    std::shared_ptr<ThreadPoolExecutor> exe):
        : acceptorFactory_(acceptorFactory)
        , socketFactory_(socketFactory)
        , exec_(exe)
        , worker_(std::make_shared<WorkMap>()){}
    ~ServerWorkerPool();

    void threadStarted(ThreadPtr thread);
    void threadStoped(ThreadPtr thread);
    void threadNotYetStoped(ThhreadPtr thread)
    {
        threadStoped(thread);
    }

private:
    //c++11 using could replace typedef
    //using WorkerMap = std::map<ThreadPtr, std::shared_ptr<Acceptor>>;
    typedef std::map<ThreadPtr, std::shared_ptr<Acceptor>> WorkerMap;
    WorkerMap worker_;
    std::shared_ptr<AcceptorFactory> acceptorFactory_;
    std::shared_ptr<ServerSocketFactory> socketFactory_;
    std::shared_ptr<ThreadPoolExecutor> exec_; 

};

class ServerBootstrap
{
public:
    ServerBootstrap();
    ~ServerBootstrap();

    void bind(int port);
    void bind(libext::SocketAddr& addr);
    void group();

    void ServerBootstrap::group(std::shared_ptr<libext::ThreadPoolExecutor> io_group,
            std::shared_ptr<libext::ThreadPoolExecutor> accept_group);

    void ServerBootstrap::group(std::shared_ptr<libext::ThreadPoolExecutor> io_group);

private:
    std::vector<std::shared_ptr<AsyncSocketBase>> sockets_;
    std::shared_ptr<ServerSocketFactory> socketFactory_;
    std::shared_ptr<ServerAcceptorFactory> acceptorFactory_;
    std::shared_ptr<ServerWorkerPool> workFactory_;
    std::shared_ptr<libext::ThreadPoolExecutor> io_group_;
    std::shared_ptr<libext::ThreadPoolExecutor> accept_group_;
};


}//libext
