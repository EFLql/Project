#include <libext/boostrap/codec/LineBasedFrameDecoder.h>

namespace libext
{
LineBasedFrameDecoder::LineBasedFrameDecoder(
                                            uint32_t maxLength, 
                                            bool stripDelimiter, 
                                            TerminatorType terminatorType)
    : maxLength_(maxLength)
    , stripDelimiter_(stripDelimiter_)
    , terminatorType_(terminatorType)
{
}

bool LineBasedFrameDecoder::decode(Context* ctx, 
                                   libext::IOBufQueue& buf, 
                                   std::unique_ptr<IOBuf>& result, 
                                   size_t& needed)
{
    int64_t eol = findEndOfLine(buf);
    if(!discarding_)
    {
        if(eol > maxLength_)
        {

        }
    }
}

int64_t LineBasedFrameDecoder::findEndOfLine(libext::IOBufQueue& buf)
{
    
}

void LineBasedFrameDecoder::fail(Context* ctx, std::string len)
{

}



}
