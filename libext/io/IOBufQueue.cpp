#include <libext/io/IOBufQueue.h>

using std::make_pair;
using std::pair;
using std::unique_ptr;

namespace
{
const int MAX_PACK_COPY = 4096;

void appendToChain(unique_ptr<IOBuf>& dst, unique_ptr<IOBuf>&& src, bool pack)
{
	if(dst == NULL)
	{
		dst = std::move(src);
	}
	else
	{
		IOBuf* tail = dst->prev();
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
pair<void*, uint64_t>
IOBufQueue::preallocateSlow(uint64_t min, 
							uint64_t newAlocationSize, 
							uint64_t max)
{
	unique_ptr<IOBuf> newBuf(IOBuf::create(std::max(min, newAlocationSize)));
	//插入到链表尾部
	appendToChain(head_, std::move(newBuf), false);
	IOBuf* last = head_->prev();
	return make_pair(last->writeableTail(), last->tailroom());
}

}