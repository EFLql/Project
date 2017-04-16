#pragma once
#include <libext/asyn/EventBase.h>
#include <libext/asyn/AsyncTransport.h>
#include <libext/asyn/EventHandler.h>
#include <libext/asyn/EventBase.h>
#include <memory>

namespace libext
{

class AsyncSocket : public AsyncTransport
{
public:
    //和shared_ptr不同，unique_ptr不能被多个unique_ptr对象共享
    typedef std::unique_ptr<AsyncSocket> UniquePtr;
    AsyncSocket(EventBase* evb, int fd);
    AsyncSocket();
    ~AsyncSocket();
    void setMaxReadsPerEvent(uint16_t maxReads)
    {
        maxReadsPerEvent_ = maxReads;
    }
    void setReadCB(ReadCallback* readCB) override;

    class IOHandler : public EventHandler
    {
    public:
        IOHandler(EventBase* evb, int fd, AsyncSocket* socket)
            : EventHandler(evb, fd)
            , socket_(socket)
        {
        }
        ~IOHandler() = default;
        void handlerReady(int16_t event) override
        {
            socket_->ioReady(event);
        }
    private:
        AsyncSocket* socket_;
    };
    void ioReady(int16_t event);

    enum class StateEnum : uint8_t
    {
        UNINT,
        CONNECTING,
        ESTABLISHED,
        CLOSED,
        ERROR,
        FAST_OPEN,
    };

    bool updateEventRegistration();
    void invalidState(ReadCallback* callback);
    void attacheEventBase(EventBase* base);
    void detachEventBase();
private:
    uint16_t maxReadsPerEvent_;
    ReadCallback* readCallback_;
    //underlying file descriptor
    int fd_;
    EventBase* eventBase_;
    IOHandler ioHandler_;
    StateEnum state_;
    int16_t eventFlags_;
};

} //libext
