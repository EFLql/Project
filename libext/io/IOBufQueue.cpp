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
                memcpy(tail->writeableTail(), src->data(), n);;
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


}
