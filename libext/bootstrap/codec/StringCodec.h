#pragma once
#include <string>

namespace libext
{
class StringCodec : public Handler<std::unique_ptr<IOBuf>, 
                                   std::string, std::string,
                                   std::unique_ptr<IOBuf>>
{
public:
    typedef typename Handler<std::unique_ptr<IOBuf>, std::string,
            std::string, std::unique_ptr<IOBuf>>::Context Context;
    
    void read(Context* ctx, std::unique_ptr<IOBuf> buf) override
    {
        if(buf)
        {
            buf->coalesce();
            std::string data((const char*)buf->data(), buf->length());
            ctx->fireRead(data);
        }
    }

    void write(Context* ctx, std::string msg) override
    {
        //code for write
    }
};

}
