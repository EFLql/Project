#include <libext/io/IOBuf.h>
#include <cstddef>
#include <iostream>

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
    std::max_align_t align;//最大对齐值16,保证buff_是对齐的
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
    uint64_t length) 
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

void IOBuf::allocExtBuffer(uint64_t minCapacity,
    uint8_t** buffReturn,
    SharedInfo** infoReturn,
    uint64_t* capacityReturn)
{
    size_t mallocSize = goodExtBufferSize(minCapacity);
    uint8_t* buf = static_cast<uint8_t*>(malloc(mallocSize));
    if(buf == NULL)
    {
        throw std::bad_alloc();
    }

    initExtBuffer(buf, mallocSize, infoReturn, capacityReturn);
    *buffReturn = buf;
}

size_t IOBuf::goodExtBufferSize(uint64_t minCapacity)
{
    size_t minSize = static_cast<size_t>(minCapacity) + sizeof(SharedInfo);
    //增加填充空间保证SharedInfo是8字节对齐的(flagsAndSharedInfo要求SharedInfo
    //起始地址是8字节对齐的,SharedInfo本身也是8字节对齐），比如如果minSize % 8==0
    //则不必填充,minSize % 8 == 1则需要填充额外的7字节依次类推
    //算法相当于8进制向前进一位，然后清除掉尾数
    minSize = (minSize + 7) & ~7;

    return minSize;
}

void IOBuf::initExtBuffer(uint8_t* buf, size_t mallocSize, 
    SharedInfo** infoReturn, 
    uint64_t* capacityReturn)
{
    //SharedInfo放到分配的buf空间最后面,并且构造出SharedInfo
    //由于SharedInfo本身也是8字节对齐，所以此时SharedInfo起始
    //地址也是8字节对齐
    uint8_t* infoStart = (buf + mallocSize) - sizeof(SharedInfo);
    SharedInfo* sharedInfo = new (infoStart) SharedInfo;

    *capacityReturn = infoStart - buf;
    *infoReturn = sharedInfo;
}

void IOBuf::freeExtBuffer()
{
    SharedInfo* info = sharedInfo();
    DCHECK(info);
    if(info->freeFn)
    {
        try
        {
            info->freeFn(buff_, info->userData);
        }
        catch(...)
        {
            //info->freeFn不应该抛出异常，否则
            //就会从析构函数抛出异常
            //同样decrementrefcount也不应该抛出异常
            abort();
        }
    }
    else
    {
        free(buff_);
    }
}

IOBuf::IOBuf(CreateOp, uint64_t capacity)
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

IOBuf::IOBuf(IOBuf&& other) noexcept
: data_(other.data_)
, buff_(other.buff_)
, length_(other.length_)
, capacity_(other.capacity_)
, flagsAndSharedInfo_(other.flagsAndSharedInfo_)
{
    other.data_ = NULL;
    other.buff_ = NULL;
    other.capacity_ = 0;
    other.length_ = 0;
    other.flagsAndSharedInfo_ = NULL;
}

/*IOBuf& IOBuf::operator=(IOBuf&& other) noexcept
{
    std::cout<<"IOBuf::IOBuf move operator"<<std::endl; 
    return *this;
}*/

/*IOBuf::IOBuf(const IOBuf& other) noexcept
{
    std::cout<<"IOBuf::IOBuf(const IOBuf)"<<std::endl;
}*/

/*IOBuf& IOBuf::operator=(const IOBuf& other) noexcept
{
    std::cout<<"operator= "<<std::endl;
    return *this;
}*/

IOBuf::~IOBuf()
{
    //析构整个IOBuf链表
    while(next_ != this)
    {
        //自动删除unlink的元素
        (void)next_->unlink();
    }
    decrementRefCount();
}

std::unique_ptr<IOBuf> IOBuf::create(uint64_t capacity)
{
    if(capacity <= kDefaultCombinedBufSize)	
    {
        return createCombined(capacity);
    }
    //IOBuf空间和底层buffer空间分开malloc出来的
    return createSeparate(capacity);
}

std::unique_ptr<IOBuf> IOBuf::createCombined(uint64_t capacity)
{
    //将IOBuf对象空间和底层buffer空间通过一次malloc操作分配空间
    //可以节省一次malloc操作
    size_t requiredStorage = offsetof(HeapFullStorage, align) + capacity;
    //lql-note:commet: Using jemalloc in wangle of facebook
    //size_t mallocSize = goodMalloc(requiredStorage);
    auto* storage = static_cast<HeapFullStorage*>(malloc(requiredStorage));

    new (&storage->hs.prefix) HeapPrefix(kIOBufInUse | kDataInUse);
    new (&storage->shared) SharedInfo(freeInternalBuf, storage);

    uint8_t* bufAddr = reinterpret_cast<uint8_t*>(&storage->align);
    uint8_t* bufEnd = reinterpret_cast<uint8_t*>(storage) + requiredStorage;
    size_t actualCapacity = bufEnd - bufAddr;
    std::unique_ptr<IOBuf> ret(new (&storage->hs.buf) IOBuf(
        InteralConstructor(), packFlagsAndSharedInfo(0, &storage->shared),
        bufAddr, actualCapacity, bufAddr, 0));

    return ret;
}

std::unique_ptr<IOBuf> IOBuf::createSeparate(uint64_t capacity)
{
    return std::make_unique<IOBuf>(CREATE, capacity);
}

void IOBuf::prependChain(std::unique_ptr<IOBuf>&& iobuf)
{
    IOBuf* other = iobuf.release();
    IOBuf* otherTail = other->prev_;
    prev_->next_ = other;
    other->prev_ = prev_;
    otherTail->next_ = this;
    prev_ = otherTail;
}

void IOBuf::releaseStorage(HeapStorage* storage)
{
    //????
}

void IOBuf::freeInternalBuf(void* buf, void* userData)
{
    HeapStorage* storage = static_cast<HeapStorage*>(userData);
    releaseStorage(storage);
}

//共享同一个底层buf内存
std::unique_ptr<IOBuf> IOBuf::cloneOne() const
{
    return std::make_unique<IOBuf>(cloneOneAsValue());    
}

IOBuf IOBuf::cloneOneAsValue() const
{
    if(SharedInfo* info = sharedInfo())
    {
        setFlags(kFlagMaybeShared);
        //用std::memory_order_acq_rel内存模型,acq_rel是一种read-modify-write操作
        //操作结果对其他线程有内存可见性(同步)
        info->refcount.fetch_add(1, std::memory_order_acq_rel);
    }
    //flagsAndSharedInfo_是一个指向共享信息的指针
    //，所有共享IOBuf对象拥有同一个共享信息指针
    return IOBuf(
            InteralConstructor(),
            flagsAndSharedInfo_,
            buff_,
            capacity_,
            data_,
            length_);
}

void IOBuf::coalesceSlow()
{
    DCHECK(isChained());
    uint64_t newLength = 0;
    IOBuf* end = this;
    do
    {
        newLength += end->length_;
        end = end->next();
    } while(end != this);

    coalesceAndReallocate(newLength, end);

    DCHECK(!isChained());
}

void IOBuf::coalesceSlow(size_t maxLength)
{
}

void IOBuf::coalesceAndReallocate(
        size_t newHeadroom,
        size_t newLength,
        IOBuf* end,
        size_t newTailRoom)
{
    uint64_t newCapacity = newHeadroom + newLength + newTailRoom;

    //allocate space for new coalesce buffer
    uint8_t* newBuf;
    SharedInfo* newInfo;
    uint64_t actualCapacity = 0;
    allocExtBuffer(newCapacity, &newBuf, &newInfo, &actualCapacity);
    
    //copy data into the new buffer
    uint8_t* newData = newBuf + newHeadroom;
    uint8_t* p = newData;

    IOBuf* current = this;
    uint64_t remainning = newLength;
    do
    {
        assert(current->length_ <= remainning);
        memcpy(p, current->data_, current->length_);
        remainning -= current->length_;
        p += current->length_;
        current = current->next_;
    } while(current != end);
    //因为条件不是以remainning为标准，所以这里需要检查
    assert(remainning == 0);
    
    decrementRefCount();

    setFlagAndSharedInfo(0, newInfo);

    capacity_ = newCapacity;
    buff_ = newBuf;
    length_ = newLength;
    data_ = newData;

    if(!isChained())
    {
        (void)separateChain(next_, current->prev_);
    }
}

void IOBuf::decrementRefCount()
{
    SharedInfo* info = sharedInfo();
    if(!info)
    {
        return;
    }

    uint32_t refCount = info->refcount.fetch_sub(
            1, std::memory_order_acq_rel);
    if(refCount > 1)//注意：这里fetch_sub返回是作减法操作之前的值
    {
        return;
    }

    //free buffer
    freeExtBuffer();

    if(flags() & kFlagFreeSharedInfo)
    {
        delete sharedInfo();
    }
}

}//libext
