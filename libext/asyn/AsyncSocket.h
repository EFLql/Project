#pragma once
#include <libext/asyn/EventBase.h>

namespace libext
{

class AsyncSocket
{
public:
    AsyncSocket(EventBase* evb, int fd);
    ~AsyncSocket();
};

} //libext
