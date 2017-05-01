#pragma once
#include <algorithm>

namespace libext
{
/*
 * IOBuf是一个以buf对象组成的双向链表其结构如下
 * +--------+
 * |  IOBuf |
 * +--------+
 *  /
 * |
 * v
 * +----------+-----------------+-----------+
 * | headroom |      data       | tailroom  |
 * +----------------------------------------+
 * ^          ^                 ^           ^
 * buff()     data()            tail()      buffEnd()
 * 同时IOBuf内部还维护了两个指针分别指向当前IOBuf链表中前一个元素
 * 与当前IOBuf链表中后一个元素
 */
class IOBuf
{
public:
    enum CreateOp { CREATE };
    enum WrapBufferOp { WRAP_BUFFER };
    enum TakeOwnershipOp { TAKE_OWNERSHIP };
    enum CopyBufferOp { COPY_BUFFER };

    typedef (*FreeFunction)(void* buff, void* userData);

    static std::unique_ptr<IOBuf> create(uint64_t capacity);
    IOBuf(CreateOp, uint64_t capacity);
   
    //IOBuf对象和实际的buff在同块连续的内存块中，一次性分配 
    static std::unique_ptr<IOBuf> createCombined(uint64_t capacity);
    //IOBuf对象和实际的buff在不同的内存块中，需要两次分配内存空间
    static std::unique_ptr<IOBuf> createSeparate(uint64_t capacity);

    //创建IOBuf链表
    static std::unique_ptr<IOBuf> createChain(
            size_t totalCapacity, uint64_t maxBufCapacity);
   
    //创建一个IOBuf指向已经存在的data buff 
    static std::unique_ptr<IOBuf> takeOwnership(void* buff, uint64_t capacity,
                                                FreeFunction freeFn = NULL,
                                                void* userData = NULL,
                                                bool freeOnErr = true)
    {
       return takeOwnership(buff, capacity, capacity, freeFn, 
               userData, freeOnErr);
    }
    IOBuf(TakeOwnershipOp op, void* buff, uint64_t capacity, 
            FreeFunction freeFn = NULL, void* userData, bool freeOnErr = true);

    static std::unique_ptr<IOBuf> takeOwnership(void* buff, uint64_t capacity,
                                                uint64_t length,
                                                FreeFunction freeFn = NULL,
                                                void* userData = NULL,
                                                bool freeOnErr = true);
    IOBuf(TakeOwnershipOp op, void* buff, uint64_t capacity, uint64_t length,
            FreeFunction freeFn = NULL, void* userData, bool freeOnErr = true);

    //创建一个IOBuf对象指向一个已经存在user-owner buffer 
    //这个函数仅仅只有在调用者提前知道创建的这个IOBuf的生命周期，
    //而且其他共享者在这个IOBuf对象之前销毁的情况下才能使用
    static std::unique_ptr<IOBuf> wrapBuffer(const void* buff, uint64_t capacity);
    //static std::unique_ptr<IOBuf> wrapBuffer(ByteRange br);//需要实现ByteRange
    static IOBuf wrapBufferAsValue(const void* buff, uint64_t capacity);
    //static IOBuf wrapBufferAsValue(ByteRange br);
    IOBuf(WrapBufferOp op, void* buff, uint64_t capacity);
    //IOBuf(WrapBufferOp, ByteRange br);
    
    //创建一个IOBuf，其数据从用户提供的buffer中拷贝,并且可配置分配头大小和尾部大小
    static std::unique_ptr<IOBuf> copyBuffer(const void* buff, uint64_t capacity,
                                             uint64_t headroom = 0,
                                             uint64_t tailroom = 0);
    //static std::unique_ptr<IOBuf> copyBuffer(ByteRange br,
    //                                         uint64_t headroom = 0,
    //                                         uint64_t tailroom = 0);
    IOBuf(CopyBufferOp op, const void* buff, uint64_t capacity, 
            uint64_t headroom = 0, uint64_t tailroom = 0);

    static std::unique_ptr<IOBuf> copyBuffer(const std::string& buff,
                                             uint64_t headroom = 0,
                                             uint64_t tailroom = 0);
    IOBuf(CopyBufferOp op, const std::string& buff, 
            uint64_t headroom = 0, uint64_t tailroom = 0)
        :IOBuf(op, buff.data(), buff.size(), headroom, tailroom) {}

    static void destory(std::unique_ptr<IOBuf>&& data)
    {
        auto destoryer = std::move(data);
    }

    ~IOBuf();
    
    bool empty() const;

    const uint8_t* data() const
    {
        return data_;
    }
    
    uint8_t* writeableData()
    {
        return data_;
    }

    const uint8_t* tail() const
    {
        return data_ + length_;
    }

    uint8_t* writeableTail()
    {
        return data_ + length_;
    }

    uint64_t length() const
    {
        return length_;
    }
    
    uint64_t headroom() const
    {
        return data_ - buffer();
    }
    
    uint64_t tailroom() const
    {
        return buffEnd() - tail();
    }

    const uint8_t* buffer() const
    {
        return buff_;
    }
    
    uint8_t* writeableBuffer()
    {
       return buff_;
    }

    const uint8_t* buffEnd() const
    {
        return buff_ + capacity_;
    }

    uint64_t capacity() const
    {
        return capacity_;
    }

    IOBuf* next()
    {
        return next_;
    }
    const IOBuf* next() const
    {
        return next_;
    }
    IOBuf* prev()
    {
        return prev_;
    }
    const IOBuf* prev() const
    {
        return prev_;
    }
    
    //将数据区整体往后移动
    void advance(int amount)
    {
        assert(amount <= tailroom());
        if(length_ > 0)
        {
            memmove(data_ + amount, data_, length_);
        }
        data_ += amount;
    }
    //将数据区整体往前移动
    void retreat(int amount)
    {
        assert(amount <= headroom());
        if(length_ > 0)
        {
            memmove(data_ - amount, data_, length_);
        }
        data_ -= amount;
    }
    
    //调整数据区指针，向前扩大数据区域
    void prepend(uint64_t amount)
    {
        //#define DEBUG调试模式下才检查的
        DCHECK_LE(amount, headroom());//检查amount <= headroom
        data_ -= amount;
        length_ += amount;
    }
    //调整数据区指针，向后扩大数据区域
    void append(uint64_t amount)
    {
        DCHECK_LE(amount, tailroom());
        length_ += amount;
    }
    
    //调整数据区起始位置
    void trimStart(uint64_t amount)
    {
        DCHECK_LE(amount, headroom());
        data_ -= amount;
        length_ += amount;
    }
    //调整数据区结束位置
    void trimEnd(uint64_t amount)
    {
        DCHECK_LE(amount, tailroom());
        length_ += amount;
    }
    
    void clean()
    {
        data_ = writeableBuffer();
        length_ = 0;
    }

    bool isChained() const
    {
        return next_ != this;
    }
    //链表长度
    size_t countChainElements() const;
    //获取整个链表中数据的长度
    uint64_t computeChainDataLength() const;
    
    //在本IOBuf对象前插入新的iobuf对象
    void prependChain(std::unique_ptr<IOBuf>&& iobuf);
    void appendChain(std::unique_ptr<IOBuf>&& iobuf)
    {
        //虽然iobuf传入进来的是右值，但是到函数里面
        //iobuf将自动变成左值
        next_->prependChain(std::move(iobuf));
    }
    
    //从当前链表中删除本IOBuf对象
    std::unique_ptr<IOBuf> unlink()
    {
        next_->prev_ = prev_;
        prev_->next_ = next_;
        next_ = this;
        prev_ = this;
        return std::unique_ptr<IOBuf>(this);
    }
    
    //从当前链表中删除本IOBuf对象，并返回删除后的链表
    std::unique_ptr<IOBuf> pop()
    {
        IOBuf* next = next_;
        next_->prev_ = prev_;
        prev_->next_ = next_;
        next_ = this;
        prev_ = this;
        return std::unique_ptr<IOBuf>(next == this ? NULL : next);
    }
    
    //从当前链表删除子串，开头为head, 结尾为tail
    //返回分离出的子串
    std::unique_ptr<IOBuf> separateChain(IOBuf* head, IOBuf* tail)
    {
        assert(head != this);
        assert(tail != this);

        head->prev_->next_ = tail->next_;
        tail->next_->prev_ = head->prev_; 
        head->prev_ = tail;
        tail->next_ = head;
        return std::unique_ptr<IOBuf>(head); 
    }
    
    struct SharedInfo
    {
        SharedInfo();
        SharedInfo(FreeFunction fn, void* arg);
        //当共享引用计数变为0的时候，
        //释放掉buffer内存空间
        FreeFunction freeFn;
        void* userData;
        //共享引用计数
        std::atomic<uint32_t> refcount;
        bool externallyShared{false};
    };

    void* operator new(size_t size);
    void* operator new(size_t size, void* ptr);
    void operator delete(void* ptr);

    //helper struct for use by operator new and delete
    //可以用于一次性为IOBuf对象和其buffer分配内存空间
    //save one malloc
    struct HeapPrefix;
    struct HeapStorage;
    struct HeapFullStorage;
private:
    uint8_t* buff_{NULL};//指向整个buff的起始位置
    uint8_t* data_{NULL};//指向buff区域的data起始位置
    uint64_t length_{0};//data区域的长度
    uint64_t capacity_{0};//整个buff的长度

    IOBuf* next_{this};
    IOBuf* prev_{this};
};

}
