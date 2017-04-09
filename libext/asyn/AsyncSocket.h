#pragma once
#include <libext/asyn/EventBase.h>
#include <libext/asyn/AsyncTransport.h>
#include <memory>

namespace libext
{

class AsyncSocket : public AsyncTransport
{
public:
    //和shared_ptr不同，unique_ptr不能被多个unique_ptr对象共享
    typedef std::unique_ptr<AsyncSocket> UniquePtr;
    AsyncSocket(EventBase* evb, int fd);
    ~AsyncSocket();
    void setMaxReadsPerEvent(uint16_t maxReads)
    {
        maxReadsPerEvent_ = maxReads;
    }
private:
    uint16_t maxReadsPerEvent_;
};

} //libext
