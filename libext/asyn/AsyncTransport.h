#pragma once
#include <memory>

namespace libext
{
class AsyncTransport
{
public:
    virtual ~AsyncTransport() = default;
    typedef std::unique_ptr<AsyncTransport> UniquePtr;
   
    //回调抽象类 
    class ReadCallback
    {
    public:
        virtual ~ReadCallback() = default;
        
        //callbacks 
        //指针在ReadCallback对象中分配内存空间
        //这样是否也是在ReadCallback对象中释放
        //而不需要在外面释放
        virtual void getReadBuff(void** bufReturn, size_t* lenRetrun) = 0;
        virtual void readDataAvailable(size_t len) = 0; 
        virtual void readEOF() = 0;
    };

    class WriteCallback
    {
    public:
        virtual ~WriteCallback() = default;
    };
    //将整个对象指针注册到该类，是由于该对象指针包含多个
    //回调函数，所以不是设置单独的回调函数，而是注册对象
    //指针
    virtual void setReadCB(ReadCallback* readCB) = 0;
};

}
