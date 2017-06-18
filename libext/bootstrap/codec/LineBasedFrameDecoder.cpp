#include <libext/boostrap/codec/LineBasedFrameDecoder.h>
#include <libext/io/Cursor.h>

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
    CHECK(eol < maxLength_);
    if(!discarding_)
    {
        if(eol >= 0)
        {
            Cursor c(buf.front());
            c += eol;
            size_t delimLength = c.read<char>() == '\r' ? 1 : 2;
            std::unique_ptr<IOBuf> frame;
            if(stripDelimiter_)//是否跳过分隔符
            {
                frame = buf.split(eol);
                buf.trimStart(delimLength);
            }
            else
            {
                frame = buf.split(eol + delimLength);
            }
            result = std::move(frame);
            return true;
        }
        else
        {
            auto len = buf.chainLength();
            if(len > maxLength_)//??
            {
                discardedBytes_ = len;
                buf.trimStart(len);
                discarding_ = true;
                char error[10] = {0};
                sprintf_s(error, sizeof(error) - 1, "over %d", len);
                fail(ctx, std::string(error));
            }
            return false;
        }
    }
    else
    {
        if(eol >= 0)
        {
            Cursor c(buf.front());
            c += eol;
            size_t delimLength = c.read<char>() == '\r' ? 2 : 1;
            buf.trimStart(eol + delimLength);
            discardedBytes_ = 0;
            discarding_ = false;
        }
        else
        {
            discardedBytes_ = buf.chainLength();
            buf.move();//整个buf丢弃
        }
        return false;
    }
}

int64_t LineBasedFrameDecoder::findEndOfLine(libext::IOBufQueue& buf)
{
   Cursor c(buf.front());
   for(uint32_t i = 0; i < maxLength_ && i < buf.chainLength(); i ++)
   {
       //c.read函数在读取的空间大于buf内剩余的空间会抛出异常
       //由程序员保证不要越界
       auto b = c.read<char>();
       if(b == '\n' && terminatorType_ != TerminatorType::CARRIAGENEWLINE)
       {
           return i;
       }
       else if(terminatorType_ != TerminatorType::NEWLINE && 
               b == '\r' && !c.isAtEnd() && c.read<char>() == '\n')
       {
           return i;
       }
   }
   return -1;
}

void LineBasedFrameDecoder::fail(Context* ctx, std::string len)
{

}



}
