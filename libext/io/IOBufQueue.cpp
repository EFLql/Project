#include <libext/io/IOBufQueue.h>

namespace
{
const int MAX_PACK_COPY = 4096;

void appendToChain(std::unique_ptr<libext::IOBuf>& dst, 
        std::unique_ptr<libext::IOBuf>&& src, bool pack)
{
    if(dst == NULL)
    {
        dst = std::move(src);
    }
    else
    {
        libext::IOBuf* tail = dst->prev();
        if(pack)//将新插入的数据插入到原来链表的尾部
        {
            size_t copyRemaining = MAX_PACK_COPY;
            uint64_t n;
            while(src && (n = src->length()) < copyRemaining &&
                n < tail->tailroom())
            {
                memcpy(tail->writeableTail(), src->data(), n);
                tail->append(n);
                copyRemaining -= n;
                src = src->pop();
            }
        }
        else
        {
            dst->appendChain(std::move(src));
        }
    }

}

} //namespace anonymous

namespace libext
{

IOBufQueue::IOBufQueue()
: chainLength_(0)
{

}

IOBufQueue::IOBufQueue(IOBufQueue&& other) noexcept
: chainLength_(other.chainLength_)
, head_(std::move(other.head_))
{
    other.chainLength_ = 0;
}

IOBufQueue& IOBufQueue::operator=(IOBufQueue&& other)
{
    if(&other != this)
    {
        chainLength_ = other.chainLength_;
        head_ = std::move(other.head_);
        other.chainLength_ = 0;
    }
    return *this;
}

std::pair<void*, uint64_t>
IOBufQueue::preallocateSlow(uint64_t min,
            uint64_t newAlocationSize, uint64_t max)
{
    std::unique_ptr<IOBuf> newBuf(IOBuf::create(std::max(min, newAlocationSize)));
    //插入到链表尾部
    appendToChain(head_, std::move(newBuf), false);
    IOBuf* last = head_->prev();
    return std::make_pair(last->writeableTail(),
            std::min(max, last->tailroom()));
}

//需由调用程序员保证不超过当前队列空间
void IOBufQueue::trimStart(size_t amount)
{
    while(amount > 0)
    {
        if(!head_)
        {
            throw std::underflow_error(
                "Attempt to trim more bytes than are present in IOBufQueue");
        }
        if(head_->length() > amount)
        {
            head_->trimStart(amount);
            chainLength_ -= amount;
            break;
        }
        amount -= head_->length();
        chainLength_ -= head_->length();
        head_ = head_->pop();//丢弃掉该IOBuf
    }
}

std::unique_ptr<IOBuf> IOBufQueue::split(size_t n)
{
    std::unique_ptr<IOBuf> result;
    while(n > 0)
    {
        if(!head_)
        {
            throw std::underflow_error(
                "Attempt to split more bytes than are present in IOBufQueue");
        }
        if(head_->length() <= n)
        {
            n -= head_->length();
            chainLength_ -= head_->length();
            std::unique_ptr<IOBuf> remainder = head_->pop();
            appendToChain(result, std::move(head_), false);
            head_ = std::move(remainder);
        }
        else//head_->length() > n
        {
            std::unique_ptr<IOBuf> clone = head_->cloneOne();
            clone->trimEnd(head_->length() - n);//tailroom
            appendToChain(result, std::move(clone), false);
            head_->trimStart(n);//去除n个字节
            chainLength_ -= n;
            break;
        }
    }
    return result;
}

}
