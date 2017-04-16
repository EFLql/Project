#pragma once
#include <libext/asyn/AsyncSocket.h>
#include <libext/bootstrap/channel/Handler.h>

namespace libext
{
//R,W handler
class AsyncSocketHandler 
    : public BytesToBytesHandler 
    , public AsyncTransport::ReadCallback
{
public:
    AsyncSocketHandler(std::shared_ptr<AsyncTransport> socket) :
        socket_(socket)
    {
    }
    void attachReadCallback()
    {
        socket_->setReadCB(this);
    }

    //handler override
    void transportActive(Context* ctx) override
    {
        attachReadCallback();
    }
    void transportInActive(Context* ctx) override
    {

    }

    void readEOF(Context* ctx) override
    {
    }

    void write(Context* ctx, win msg) override
    {
    }

    //ReadCallback override
    void getReadBuff(void** bufReturn, size_t* lenRetrun) override
    {
    }

    void readDataAvailable(size_t len) override
    {
    }
private:
    std::shared_ptr<AsyncTransport> socket_;
};

} //libext
