#pragma once
#include <libext/io/IOBuf.h>
#include <cstring>
#include <stdexcept>

namespace detail
{
//CursorBase对象相当于指向IOBuf链表上面某个特定的IOBuf
template <class Derived, class BufType>
class CursorBase
{
    //编译时警告也不会产生，因为模板参数不通确实属于不通的类定义了
    //如果该类没有模板参数，则用它自己为友员编译是会警告
    template <class D, class B> friend class CursorBase;
public:
    explicit CursorBase(BufType* buf) : crtBuf_(buf), buffer_(buf) {}

    template<class OtherDerived, class OtherBuf>
    explicit CursorBase(const CursorBase<OtherDerived, OtherBuf>& cursor)
        : crtBuf_(cursor.crtBuf_)
        , buffer_(cursor.buffer_)
        , offset_(cursor.offset_) {}

    //reset cursor to the new buffer
    void reset(BufType* buf)
    {
        crtBuf_ = buf;
        buffer_ = buf;
        offset_ = 0;
    }

    const uint8_t* data() const
    {
        return crtBuf_->data() + offset_;
    }

    //返回当前cursor所在的IOBuf对象数据长度
    size_t length() const
    {
        return crtBuf_->length() - offset_;
    }

    //返回整个IOBuf链表的数据长度
    size_t totalLength() const
    {
        if(crtBuf_ == buffer_)
        {
            return crtBuf_->computeChainDataLength() - offset_;
        }
        CursorBase end(buffer_->pre());//链表尾
        end.offset_ = end.buffer_->length();//偏移到最后
        return end - *this;
    }
   
    //这个函数用来检查当前链表空间 
    bool canAdvance(size_t amount) const
    {
        const IOBuf* nextBuf = crtBuf_;
        size_t available = nextBuf->length();
        do
        {
            if(amount > available)
            {
                return false;
            }
            amount -= available;
            nextBuf = nextBuf->next();
        } while(nextBuf != buffer_);
        return true;
    }
    
    Derived& operator+=(size_t offset)
    {
        Derived* p = static_cast<Derived*>(this);
        p->skip(offset);
        return *p;
    }

    Derived operator+(size_t offset)
    {
        Derived other(*this);
        other.skip(offset);
        return other;
    }

    void skip(size_t len)
    {
        if(length() >=len)
        {
           offset_ += len;
           advanceBufferIfEmpty();
        }
        else
        {
            skipSlow(len);
        }
    }
    
    //两个cursor的距离
    size_t operator-(const CursorBase& other)//other指向同一个IOBuf链表中不同的IOBuf对象
    {
        BufType* otherBuf = other.crtBuf_;
        size_t distance = 0;
        if(otherBuf != crtBuf_)
        {
            len += other.length();
            for(otherBuf = otherBuf->next; 
                otherBuf != crtBuf_ && otherBuf != other.buffer_;
                otherBuf = otherBuf->next())
            {
                len += otherBuf->length();
            }
            if(otherBuf == other.buffer_)
            {
                throw std::out_of_range("wrap-around");
            }
            len += offset_;
        }
        else
        {
            if(offset_ < otherBuf.offset_)
            {
                throw std::out_of_range("underflow");
            }
            len += offset_ - otherBuf.offset_;
        }
        return len;
    }

    template <class T>
    typename std::enable_if<std::is_arithmetic<T>::value, T>::type read()//std::enable_if如果条件不满足编译会报错
    {
        T val;
        if(length() >= sizeof(T))
        {
            memcpy(&val, data(), sizeof(T));
            offset_ += sizeof(T);
            advanceBufferIfEmpty();
        }
        else
        {
            pullSlow(&val, sizeof(T));
        }
        return val;
    }

protected:
    bool tryAdvanceBuffer()
    {
        const BufType* nextBuf = crtBuf_->next();
        if(nextBuf == buffer_)//到达链表头
        {
            offset_ = crtBuf_->length();
            return false;
        }
        offset_ = 0;
        crtBuf_ = nextBuf;
        return true;
    }

    void advanceBufferIfEmpty()
    {
        if(length() == 0)
        {
            tryAdvanceBuffer();
        }
    }

private: 
    size_t skipAtMostSlow(size_t len)
    {
        size_t skipped = 0;
        for(size_t available; (available = length()) < len; )
        {
            skipped += available;
            if(!tryAdvanceBuffer())
            {
                return skipped;
            }
            len -= available;
        }
        offset_ += len;
        advanceBufferIfEmpty();
        return skipped + len;
    }

    void skipSlow(size_t len)
    {
        if(skipAtMostSlow(len) != len)
        {
            throw std::out_of_range("underflow");
        }
    }
    
    size_t pullAtMostSlow(void* buf, size_t len)
    {
        uint8_t* p = reinterpret_cast<uint8_t*>(buf);
        size_t coped = 0;
        for(size_t available; (available = length()) < len; )
        {
            coped += avaiable;
            memcpy(p, data(), available);
            if(!tryAdvanceBuffer())
            {
                return coped;
            }
            p += available;
            len -= available;
        }
        memcpy(p, data(), len);
        offset_ += len;
        advanceBufferIfEmpty();
        return coped + len;
    }

    void pullSlow(void* buf, size_t len)
    {
        if(pullAtMostSlow(buf, len) != len)
        {
            throw std::out_of_range("underflow");
        }
    }

private:
    BufType* crtBuf_;//链表中当前的buf位置
    size_t offset_;

    BufType* buffer_;//链表头
};

} //detail
