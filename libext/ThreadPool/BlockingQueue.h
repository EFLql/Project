#pragma once
#include <libext/typedef.h>

namespace libext
{

template <class T>
class BlockingQueue
{
public:
    BlockingQueue() {};
    virtual ~BlockingQueue() {}

    virtual void add(T item) = 0;
    virtual void addWithPriority(T item, int8_t priority)
    {
        add(std::move(item));
    }
    
    virtual T take() = 0;
    virtual size_t size() = 0;
};


} //libext
