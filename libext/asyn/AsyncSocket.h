#pragma once
#include <libext/asyn/EventBase.h>
#include <libext/asyn/AsyncTransport.h>
#include <libext/asyn/EventHandler.h>
#include <libext/asyn/EventBase.h>
#include <libext/asyn/AsyncSocketException.h>
#include <libext/SocketAddr.h>
#include <memory>
#include <string>
#include <assert.h>

namespace libext
{

class AsyncSocket : public AsyncTransport
{
public:
    //和shared_ptr不同，unique_ptr不能被多个unique_ptr对象共享
    typedef std::unique_ptr<AsyncSocket> UniquePtr;
    AsyncSocket(EventBase* evb, int fd);
    AsyncSocket(EventBase* evb);
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

    struct WriteResult
    {
        explicit WriteResult(ssize_t ret) : writeReturn(ret) {}

        explicit WriteResult(ssize_t ret,
                std::unique_ptr<AsyncSocketException> e)
            :writeReturn(ret), exception(std::move(e)) {}
        ssize_t writeReturn;
        std::unique_ptr<AsyncSocketException> exception;
    };
    //ReadResult 读取返回值
    //读取过程中不一定所有的读取错误都是异常的
    //比如读取到文件尾导致的读取失败不算异常信息
    struct ReadResult
    {
        explicit ReadResult(ssize_t ret) : readReturn(ret) {}

        explicit ReadResult(ssize_t ret,
                std::unique_ptr<AsyncSocketException> e)
            :readReturn(ret), exception(std::move(e)) {}

        ssize_t readReturn;
        std::unique_ptr<const AsyncSocketException> exception;
    };
    class WriteRequest
    {
    public:
        WriteRequest(AsyncSocket* socket, WriteCallback* callback)
            :socket_(socket), callback_(callback) {}
        virtual void start() {}
        virtual void destroy() = 0;
        virtual WriteResult performWrite() = 0;

        WriteRequest* getNext() const
        {
            return next_;
        }
        WriteCallback* getCallback() const
        {
            return callback_;
        }
        void append(WriteRequest* next)
        {
            assert(next_ == NULL);
            next_ = next;
        }
        void bytesWritten(size_t count)
        {
            totalBytesWritten_ += count;
        }
        void fail(const char* fn, const AsyncSocketException& e)
        {
            socket_->failWrite(fn, e);
        }
    protected:
        AsyncSocket* socket_;
        WriteCallback* callback_;
        WriteRequest* next_{NULL};
        uint32_t totalBytesWritten_{0};
    };

    bool updateEventRegistration();
    bool updateEventRegistration(int32_t enable, int32_t disable);
    void invalidState(ReadCallback* callback);
    void invalidState(ConnectCallback* callback);
    void attacheEventBase(EventBase* base);
    void detachEventBase();
    void connect(ConnectCallback* callback, const SocketAddr& addr,
            uint32_t connectTimeout) noexcept;
    void connect(ConnectCallback* callback, const std::string& ip,
            uint16_t port, uint32_t connectTimeout) noexcept;
protected:
    enum ReadResultEnum
    {
        READ_EOF = 0,
        READ_ERROR = -1,
        READ_BLOCKING = -2,
        READ_NO_ERROR = -3,
    };

protected:
    void registerForConnectEvents();
    void handleConnect() noexcept;
    void handleRead() noexcept;
    void handleWrite() noexcept;
    void handleInitialReadWrite() noexcept;
    void prepareReadBuffer(void** buff, size_t* bufflen) noexcept;
    ReadResult performRead(void** buff, size_t* bufflen, 
            size_t* /*offset*/) noexcept;
private:
    void failConnect(const char* fn, const AsyncSocketException& ex);
    void failRead(const char* fn, const AsyncSocketException& ex);
    void failWrite(const char* fn, const AsyncSocketException& ex);
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
    WriteRequest* writeReqHead_{NULL}; //chain of write request
    WriteRequest* writeReqTail_{NULL}; //end of WriteRequest chain
};

} //libext
