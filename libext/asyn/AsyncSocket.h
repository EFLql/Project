#pragma once
#include <libext/asyn/EventBase.h>
#include <libext/asyn/AsyncTransport.h>
#include <libext/asyn/EventHandler.h>
#include <libext/asyn/EventBase.h>
#include <libext/asyn/AsyncSocketException.h>
#include <libext/SocketAddr.h>
#include <memory>
#include <string>

namespace libext
{

class AsyncSocket : public AsyncTransport
{
public:
    //和shared_ptr不同，unique_ptr不能被多个unique_ptr对象共享
    typedef std::unique_ptr<AsyncSocket> UniquePtr;
    AsyncSocket(EventBase* evb, int fd);
    AsyncSocket();
    AsyncSocket(EventBase* evb, const SocketAddr& addr,
            uint32_t connectTimeout);
    AsyncSocket(EventBase* evb, const std::string& ip,
            uint16_t port, uint32_t connectTimeout);

    ~AsyncSocket();
    class ConnectCallback
    {
    public:
        virtual ~ConnectCallback() = default;

        virtual void connectSucc() noexcept = 0;
        virtual void connectErr(
                const AsyncSocketException& ex) noexcept = 0;
    };

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
    void invalidState(ConnectCallback* callback);
    void attacheEventBase(EventBase* base);
    void detachEventBase();
    void connect(ConnectCallback* callback, const SocketAddr& addr,
            uint32_t connectTimeout) noexcept;
    void connect(ConnectCallback* callback, const std::string& ip,
            uint16_t port, uint32_t connectTimeout) noexcept;
protected:
    void registerForConnectEvents();
private:
    void failConnect(const char* fn, const AsyncSocketException& ex);
    void invokeConnectSuccess();
    int socketConnect(const SocketAddr& addr);
private:
    uint16_t maxReadsPerEvent_;
    ReadCallback* readCallback_;
    ConnectCallback* connectCallback_;
    //underlying file descriptor
    int fd_;
    SocketAddr addr_;
    EventBase* eventBase_;
    IOHandler ioHandler_;
    StateEnum state_;
    int16_t eventFlags_;
};

} //libext
