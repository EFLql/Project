#include <libext/io/IOBufQueue.h>

using std::make_pair;
using std::pair;
using std::unique_ptr;

namespace
{
void appendToChain(unique_ptr<IOBuf>& dst, unique_ptr<IOBuf>&& src, bool pack)
{

}

} //namespace anonymous

namespace libext
{
std::pair<void*, uint64_t>
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