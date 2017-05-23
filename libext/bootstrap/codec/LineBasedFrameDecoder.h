#pragma once
#include <libext/bootstrap/codec/ByteToMessageDecoder.h>
#include <string>

namespace libext
{
class LineBasedFrameDecoder : public ByteToByteDecoder
{
public:
    enum class TerminatorType
    {
        BOTH,
        NEWLINE,
        CARRIGENEWLINE,
    };
    explicit LineBasedFrameDecoder(
        uint32_t maxLength = UINT_MAX,
        bool stripDelimiter = true,
        TerminatorType terminatorType = TerminatorType::BOTH);
 
    bool decode(Context* ctx, 
                libext::IOBufQueue& buf, 
                std::unique_ptr<IOBuf>& result, 
                size_t& needed);

private:
    int64_t findEndOfLine(libext::IOBufQueue& buf);
    void fail(Context* ctx, std::string len);

    uint32_t maxLength_;
    bool stripDelimiter_{true};

    bool discarding_{false};
    uint32_t discardedBytes_{0};

    TerminatorType terminatorType_;
};

}
