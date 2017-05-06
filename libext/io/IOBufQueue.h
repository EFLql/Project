#pragma once
#include <libext/io/IOBuf.h>

namespace libext
{
/*
 * IOBufQueue 封装(encapsulate)IObuf链表，提供一系列方便
 * 的方法以供外部从链表中插入或删除数据
 */
class IOBufQueue
{
public:
    ~IOBufQueue() = default;
    //预分配内存空间
    std::pair<uint64_t, uint64_t> preallocate(
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
    void postallocate(uint64_t size);

private:
	IOBuf* tailBuf() const
	{
		if(!head_) return NULL;
		IOBuf* buf = head_->prev();
		return (!buf->isSharedOne()) ? buf : NULL;
	}
	std::pair<void*, uint64_t> preallocateSlow(uint64_t min,
		uint64_t newAlocationSize, uint64_t max);

private:
	std::unique_ptr<IOBuf> head_;
};

} //libext
