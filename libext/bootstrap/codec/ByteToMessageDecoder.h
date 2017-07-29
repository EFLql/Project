#pragma once
#include <libext/bootstrap/channel/Handler.h>
#include <libext/io/IOBufQueue.h>
#include <type_traits>
#include <memory>

namespace libext
{
template <class M>
class ByteToMessageDecoder : public InboundHandler<IOBufQueue&, M>
{
public:
    typedef typename InboundHandler<IOBufQueue&, M>::Context Context;
    //具体的decode接口
    virtual bool decode(Context* ctx, IOBufQueue& buf, M& result, size_t&) = 0;
    virtual ~ByteToMessageDecoder() = default; 
    void read(Context* ctx, IOBufQueue& buf) override
    {
        bool success = true;
        do
        {
            M result;
            size_t needed = 0;
            success = decode(ctx, buf, result, needed);
            if(success)
            {
                ctx->fireRead(std::move(result));
            }
        }while(success);
    }
};

typedef ByteToMessageDecoder<std::unique_ptr<IOBuf>> ByteToByteDecoder;

}
