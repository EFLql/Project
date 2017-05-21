#pragma once
#include <libext/io/IOBufQueue.h>

namespace libext
{
template <class M>
class ByteToMessageDecoder : public InboundHandler<libext::IOBufQueue&, M>
{
public:
    typedef typename InboundHandler<libext::IOBufQueue&, M>::Context Context;
    //具体的decode接口
    virtual bool decode(Context* ctx, libext::IOBufQueue& buf, M& result, size_t&) = 0;
    virtual ~ByteToMessageDecoder() = defalut; 
    void read(Context* ctx, libext::IOBufQueue& buf) override
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

typedef ByteToMessageDecoder<std::unique_ptr<libext::IOBuf>> ByteToByteDecoder;

}
