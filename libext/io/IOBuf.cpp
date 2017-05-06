#include <libext/io/IOBuf.h>
#include <cstddef>

namespace
{

enum : uint16_t
{
    kHeapMagic = 0xa5a5,
    kIOBufInUse = 0x01,
    kDataInUse = 0x02,
};

enum : uint64_t
{
	kDefaultCombinedBufSize = 1024
};

}//nonamed namespace

namespace libext
{

struct IOBuf::HeapPrefix
{
    HeapPrefix(uint16_t flag)
        :magic(kHeapMagic), flags(flag)
    {
    }
    ~HeapPrefix()
    {
        magic = 0;//for user debugging
    }
    //可以被任意扩展为data type,code fragment,keyword...
    //的SV(scalar value),定义时无法知道其具体含义
    //只有在实际使用时才知道
    uint16_t magic;
    std::atomic<uint16_t> flags; 
};

struct IOBuf::HeapStorage
{
    HeapPrefix prefix;
    IOBuf buf;
};

struct IOBuf::HeapFullStorage
{
    static_assert(sizeof(HeapStorage) <= 64,
            "IOBuf may not grow over 56 bytes!");
    HeapStorage hs;
    SharedInfo shared;
    std::max_align_t align;
};

IOBuf::SharedInfo::SharedInfo()
    :freeFn(NULL), userData(NULL)
{
    //使用 relaxed内存顺序(即没有任何内存屏障,
    //只保证操作的原子性)，因为我们正在创建一个
    //新的ShreadInfo对象，此时还不可能有多线程访问
    refcount.store(1, std::memory_order_relaxed);
}

IOBuf::SharedInfo::SharedInfo(FreeFunction fn, void* arg)
    :freeFn(fn), userData(arg)
{
    //使用 relaxed内存顺序(即没有任何内存屏障,
    //只保证操作的原子性)，因为我们正在创建一个
    //新的ShreadInfo对象，此时还不可能有多线程访问
    refcount.store(1, std::memory_order_relaxed);
}

void* IOBuf::operator new(size_t size)
{
    size_t fullSize = offsetof(HeapStorage, buf) + size;
    auto* storage = static_cast<HeapStorage*>(malloc(fullSize));
    if(storage == NULL)
    {
        throw std::bad_alloc();
    }

    new (&storage->prefix) HeapPrefix(kIOBufInUse);
    return &(storage->buf);
}

void* IOBuf::operator new(size_t /*size*/, void* ptr)
{
	return ptr;
}

void IOBuf::operator delete(void* ptr)
{
	//code for delete
}

IOBuf::IOBuf(InteralConstructor, 
			 uintptr_t flagsAndSharedInfo, 
			 uint8_t* buf, 
			 uint64_t capacity, 
			 uint8_t* data, 
			 uint8_t length) 
	: next_(this)
	, prev_(this)
	, buff_(buf)
	, data_(data)
	, length_(length)
	, capacity_(capacity)
	, flagsAndSharedInfo_(flagsAndSharedInfo)
{
	assert(data >= buf);
	assert(data + length <= buf + capacity);
}

std::unique_ptr<IOBuf> IOBuf::IOBuf(CreateOp, uint64_t capacity)
: next_(this)
, prev_(this)
, data_(NULL)
, length_(0)
, flagsAndSharedInfo_(0)
{
	SharedInfo* info;
	allocExtBuffer(capacity, &buff_, &info, &capacity_);
	setSharedInfo(info);
	data_ = buff_;
}

std::unique_ptr<IOBuf> IOBuf::create(uint64_t capacity)
{
	if(capacity <= kDefaultCombinedBufSize)	
	{
		return createCombined(capacity);
	}
	return createSeparate(capacity);
}

std::unique_ptr<IOBuf> IOBuf::createCombined(uint64_t capacity)
{
	size_t requiredStorage = offsetof(HeapFullStorage, align) + capacity;
	//lql-note:commet: Using jemalloc in wangle of facebook
	//size_t mallocSize = goodMalloc(requiredStorage);
	auto* storage = static_cast<HeapFullStorage*>(malloc(requiredStorage));

	new (&storage->hs.prefix) HeapPrefix(kIOBufInUse | kDataInUse);
	new (&storage->shared) SharedInfo(freeInternalBuf, storage);

	uint8_t* bufAddr = reinterpret_cast<uint8_t*>(&storage->align);
	uint8_t* bufEnd = reinterpret_cast<uint8_t*>(storage) + requiredStorage;
	size_t actualCapacity = bufEnd - bufAddr;
	unique_ptr<IOBuf> ret(new (&storage->hs.buf) IOBuf(
		InteralConstructor(), packFlagsAndSharedInfo(0, &storage->shared),
		bufAddr, actualCapacity, bufAddr, 0));

	return ret;
}

std::unique_ptr<IOBuf> IOBuf::createSeparate(uint64_t capacity)
{
	return std::make_unique<IOBuf>(CREATE, capacity);
}




}//libext
