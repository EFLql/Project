#pragma once
#include <libext/bootstrap/acceptor/Acceptor.h>
#include <libext/ThreadPool/IOThreadPoolExecutor.h>
#include <libext/bootstrap/ServerSocketFactory.h>
#include <libext/lock/SpinLock.h>
#include <libext/bootstrap/channel/Pipeline.h>
#include <map>
#include <memory>
namespace libext
{
template <typename Pipeline>
class ServerAcceptor
    : public Acceptor
    //, public InboundHandler<>
{
public:
    class ServerConnection 
        : public libext::PipelineManager
        , public ManagedConnection
    {
    public:
        ServerConnection(typename Pipeline::Ptr pipeline)
            : pipeline_(std::move(pipeline))
        {
            pipeline_->setPipelineManager(this);
        }
        ~ServerConnection()
        {
            pipeline_->setPipelineManager(NULL);
        }
        void init()
        {
            pipeline_->transportActive();
        }
        /*ManagedConnection extend....
        *连接超时，丢弃连接等
        *
        *lql-need add code...
        */
    private:
        typename Pipeline::Ptr pipeline_;//处理连接请求pipeline
    };
    ServerAcceptor(
        std::shared_ptr<PipelineFactory<DefaultPipeline>> childPipelineFactory)
        : childPipelineFactory_(childPipelineFactory)
    {
    }
    ~ServerAcceptor() = default;
    void onNewConnection(
            AsyncTransport::UniquePtr sock,
            const SocketAddr& clientAddr,
            SecureTransportType secureTransportType,
            TransportInfo& tinfo) override
    {
        std::cout<<"ServerAcceptor::onNewConnection ready"<<std::endl;
        //利用自定以的工厂创建自定义的pipeline
        auto pipeline = childPipelineFactory_->newPipeline(
                std::shared_ptr<AsyncTransport>(sock.release()));
        ServerConnection* connect = NULL;
        try
        {
            connect = new ServerConnection(std::move(pipeline));
        }catch(const std::bad_alloc& e)
        {
            //lql-ERR
            std::cout<<"new ServerConnection faild "<<e.what()<<std::endl;
            throw std::runtime_error("new ServerConnection faild");
        }
        Acceptor::addConnection(static_cast<ManagedConnection*>(connect));
        connect->init();
    }
private:
    std::shared_ptr<PipelineFactory<Pipeline>> childPipelineFactory_;
};

template <typename Pipeline>
class ServerAcceptorFactory : public AcceptorFactory
{
public:
    ServerAcceptorFactory(
        std::shared_ptr<PipelineFactory<Pipeline>> childPipelineFactory)
        : childPipelineFactory_(childPipelineFactory)
    {
    }
    ~ServerAcceptorFactory() = default;
    std::shared_ptr<Acceptor> newAcceptor(EventBase* evb) override
    {
       auto acceptor = std::make_shared<ServerAcceptor<Pipeline>>(
           childPipelineFactory_);
       acceptor->init(evb);
       return acceptor;
    }
private:
    std::shared_ptr<PipelineFactory<Pipeline>> childPipelineFactory_;
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

    void threadStarted(ThreadPoolExecutor::ThreadPtr thread);
    void threadStoped(ThreadPoolExecutor::ThreadPtr thread);
    void threadNotYetStoped(ThreadPoolExecutor::ThreadPtr thread)
    {
        threadStoped(thread);
    }
    template<typename F>
    void forEachWork(F&& f)
    {
        libext::SpinLockGuard g(spinLock_);
        for(auto& woker : *worker_)
        {
            f(woker.second.get()); 
        }
    }
private:
    //c++11 using could replace typedef
    //using WorkerMap = std::map<ThreadPtr, std::shared_ptr<Acceptor>>;
    libext::SpinLock spinLock_;
    typedef std::map<ThreadPoolExecutor::ThreadPtr, std::shared_ptr<Acceptor>> WorkerMap;
    std::shared_ptr<WorkerMap> worker_;
    std::shared_ptr<AcceptorFactory> acceptorFactory_;
    std::shared_ptr<ServerSocketFactory> socketFactory_;
    IOThreadPoolExecutor* exec_; 
    std::vector<std::shared_ptr<AsyncSocketBase>> sockets_;

};

template <typename Pipeline>
class ServerBootstrap
{
public:
    ServerBootstrap() : reusePort_(false) {}
    ~ServerBootstrap() {}
    void bind(int port);
    void bind(libext::SocketAddr& addr);
    void group(std::shared_ptr<IOThreadPoolExecutor> io_group);

    void group(std::shared_ptr<IOThreadPoolExecutor> io_group,
            std::shared_ptr<IOThreadPoolExecutor> accept_group);
    void setSocketFactor(std::shared_ptr<ServerSocketFactory> factory)
    {
        socketFactory_ = factory;
    }
    ServerBootstrap* childPipeline(
            std::shared_ptr<PipelineFactory<Pipeline>> childPipelineFactory)
    {
        childPipelineFactory_ = std::move(childPipelineFactory);
    }
private:
    std::vector<std::shared_ptr<AsyncSocketBase>> sockets_;
    std::shared_ptr<ServerSocketFactory> socketFactory_;
    std::shared_ptr<ServerAcceptorFactory<DefaultPipeline>> acceptorFactory_;
    std::shared_ptr<ServerWorkerPool> workFactory_;
    std::shared_ptr<libext::IOThreadPoolExecutor> io_group_;
    std::shared_ptr<libext::IOThreadPoolExecutor> accept_group_;
    std::shared_ptr<PipelineFactory<Pipeline>> childPipelineFactory_;
    bool reusePort_;
};

}//libext

#include <libext/bootstrap/ServerBootstrap-inl.h>
