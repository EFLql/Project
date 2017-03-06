#pragma once
#include <libext/asyn/EventBase.h>

namespace libext
{
class AsyncSocketBase
{
public:
    AsyncSocketBase() {}
    virtual ~AsyncSocketBase() {}
    virtual EventBase* getEventBase() const = 0; 
    
};

}//libext
