//test class ServerBootstrap、ServerSocketFactory

#include <gtest/gtest.h>
#include <libext/bootstrap/ServerBootstrap.h>
#include <libext/bootstrap/channel/AsyncSocketHandler.h>
#include <libext/bootstrap/codec/LineBasedFrameDecoder.h>
#include <libext/bootstrap/codec/StringCodec.h>

using namespace libext;//外部引用命名空间里面成员
typedef ServerBootstrap<DefaultPipeline> TestServer;

class EchoHandler : public HandlerAdapter<std::string>
{
public:
    virtual void read(Context* ctx, std::string msg) override
    {
        std::cout<<"recv msg: "<<msg.c_str()<<std::endl;
    }
};

class PipelineFactoryChild : public PipelineFactory<DefaultPipeline>
{
public:
    typename DefaultPipeline::Ptr newPipeline(std::shared_ptr<AsyncTransport> sock)
    {
        typename DefaultPipeline::Ptr pipeline = DefaultPipeline::create();
        pipeline->addBack(std::move(AsyncSocketHandler(sock)));
        pipeline->addBack(LineBasedFrameDecoder(8192));
        pipeline->addBack(StringCodec());
        pipeline->addBack(EchoHandler());
        pipeline->finalize();
        return pipeline;
    }
};

TEST(Bootstrap, Basic)
{
    TestServer server;
}


TEST(Bootstrap, ServerAcceptorGroupTest)
{

}

TEST(ServerSocketFactory, newSocketTest)
{
    AsyncServerSocketFactory factory;
    SocketAddr addr;//SocketAddr应该已经经过了gtest的测试
    addr.setFromLocalPort(8888);
    auto socket = factory.newSocket(addr, 5, false);

    EXPECT_TRUE(socket.get());
} 

TEST(Bootstrap, bindTest)
{
    TestServer server;
    server.setSocketFactor(std::make_shared<libext::AsyncServerSocketFactory>());
    server.childPipeline(std::make_shared<PipelineFactoryChild>());
    server.bind(8080);
    sleep(500);
}
