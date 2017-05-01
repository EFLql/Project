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

}//libext
