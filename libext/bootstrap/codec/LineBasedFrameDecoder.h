#pragma once
#include <libext/bootstrap/codec/ByteToMessageDecoder.h>

namespace libext
{
class LineBasedFrameDecoder : public ByteToByteDecoder
{
public:
    LineBasedFrameDecoder();
    ~LineBasedFrameDecoder() {}
    bool decode(Context* ctx, IOBufQueue& buf, IOBuf& result, size_t& needed); 
};

}
