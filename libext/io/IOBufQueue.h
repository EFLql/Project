#pragma once
#include <libext/typedef.h>
#include <libext/io/IOBuf.h>
#include <utility>
#include <limits>

namespace libext
{
/*
* IOBufQueue 封装(encapsulate)IObuf链表，提供一系列方便
* 的方法以供外部从链表中插入或删除数据,原始的IOBuf对象只
* 代表它本身，IOBufQueue不同它代表整个IOBuf链表
*/
class IOBufQueue
{
public:
    IOBufQueue();
    ~IOBufQueue() = default;
    //预分配内存空间
    std::pair<void*, uint64_t> preallocate(
        uint64_t min,
        uint64_t newAllocationSize,
        uint64_t max = std::numeric_limits<uint64_t>::max())
    {
        auto buf = tailBuf();
        if(buf && buf->tailroom() >= min)
        {
            return std::make_pair(buf->writeableTail(), 
                std::min(max, buf->tailroom()));
        }
        return preallocateSlow(min, newAllocationSize, max);
    }

    //实际使用的内存空间,空余的内存空间供下次使用
    void postallocate(uint64_t n)
    {
        head_->prev()->append(n);
        chainLength_ += n;
    }

    size_t getChainLength() const
    {
        return chainLength_;
    }

    //moveable
    IOBufQueue(IOBufQueue&&) noexcept;
    IOBufQueue& operator=(IOBufQueue&&);

    void trimStart(size_t amount);
    std::unique_ptr<IOBuf> split(size_t n);

    std::unique_ptr<IOBuf> move()
    {
        chainLength_ = 0;
        return std::move(head_);
    }

    const IOBuf* front() const
    {
        return head_.get();
    }

private:
    //Not copyable
    IOBufQueue(const IOBufQueue&) = delete;
    const IOBufQueue& operator=(const IOBufQueue&) = delete;
    IOBuf* tailBuf() const
    {
        if(!head_) return NULL;
        IOBuf* buf = head_->prev();
        return (!buf->isSharedOne()) ? buf : NULL;
    }
    //这个地方碰到链接出错，最后是发现没有引用typedef.h
    //明确定义uint64_t类型导致的。
    //经验:链接出错除了没有引用到正确的动态库(静态库)文件或目标文件(.o)
    //外还有可能是函数参数导致的，虽然编译可以通过
    std::pair<void*, uint64_t> preallocateSlow(uint64_t min,
            uint64_t newAlocationSize, uint64_t max);
private:
    std::unique_ptr<IOBuf> head_;
    size_t chainLength_{0};
};

} //libext
