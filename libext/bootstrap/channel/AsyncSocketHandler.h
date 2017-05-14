#pragma once
#include <libext/asyn/AsyncSocket.h>
#include <libext/bootstrap/channel/Handler.h>
#include <libext/io/IOBufQueue.h>

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
    AsyncSocketHandler(AsyncSocketHandler&&) = default;
   ~AsyncSocketHandler() = default;
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
    void readEOF() override
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
        const auto readBufferSetting = getContext()->getReadBufferSetting();
        const auto ret = buffQueue_.preallocate(
               readBufferSetting.first,
               readBufferSetting.second);
       *bufReturn = ret.first;
       *lenRetrun = ret.second;
    }

    void readDataAvailable(size_t len) override
    {
        buffQueue_.postallocate(len);
        getContext()->fireRead(buffQueue_);
    }
private:
    std::shared_ptr<AsyncTransport> socket_;
    IOBufQueue buffQueue_;
};

} //libext
