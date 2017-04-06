#pragma once
#include <libext/bootstrap/channel/HandlerContext.h>

namespace libext
{
template <class Context>
class HandlerBase
{
public:
    virtual ~HandlerBase() = default;
    virtual attachPipline(Context* /*ctx*/) {}
    virtual detachPipline(Context* /*ctx*/) {}

    Context* getContext()
    {
        if(attachCount_ != 1)
        {
            return NULL;
        }
        CHECK(ctx_);
        return ctx_;
    }
private:
    uint64_t attachCount_{0};
    Context* ctx_{NULL};
};

//Rin 表示读入操作
//Rout 表示将可读操作转发出去
//Win 表示写入操作
//Wout 表示将可写操作转发出去
template <class Rin, class Rout = Rin, class Win = Rin, class Wout = Rout>
class Handler : public HandlerBase<HandlerContext<Rout, Wout>>
{
public:
    typedef Rin rin;
    typedef Rout rout;
    typedef Win win;
    typedef Wout wout;
    typedef HandlerContext<Rout, Wout> Context;
    
    virtual ~Handler() = default;
    static const HandlerDir dir = HandlerDir::BOTH;
    
    virtual void read(Context* ctx, Rin msg) = 0;
    virtual void readEOF(Context* ctx)
    {
        ctx->fireReadEOF();//直接转发，handler不处理
    }
    virtual void transportActive(Context* ctx)
    {
        ctx->fireTransportActive();
    }
    virtual void transportInActive(Context* ctx) 
    {
        ctx->fireTransportActive();
    }
    virtual void write(Context* ctx, Win msg) = 0;
    virtual void close(Context* ctx)
    {
        ctx->fireClose();
    }
};

template <class Rin, class Rout = Rin>
class InboundHandler : public HandlerBase<InboundHandlerContext<Rout>>
{
public:
    static const HandlerDir dir = HandlerDir::IN;
    typedef Rin rin;
    typedef Rout rout;
    typedef void win;
    typedef void wout;
    typedef InboundHandlerContext<Rout> Context;

    virtual ~InboundHandler() = default;

    virtual void read(Context* ctx, Rin msg) = 0;
    virtual void readEOF(Context* ctx)
    {
        ctx->fireReadEOF();
    }
    virtual void transportActive(Context* ctx)
    {
        ctx->fireTransportActive();
    }
    virtual void transportInActive(Context* ctx)
    {
        ctx->fireTransportInActive();
    }
};

template <class Win, class Wout = Win>
class OutboundHandler : public HandlerBase<OutbundHandlerContext<Wout>>
{
public:
    static const HandlerDir dir = HandlerDir::OUT;
    typedef Win win;
    typedef Wout wout;
    typedef void rin;
    typedef void rout;
    typedef OutboundHandlerContext<Wout> Context;

    virtual ~OutboundHandler() = default;

    virtual void write(Context* ctx, Win msg) = 0;
    virtual void close(Context* ctx)
    {
        ctx->fireclose();
    }
};

template <class R, class W>
class HandlerAdapter : publc Handler<R, R, W, W>
{
public:
    typename Handler<R, R, W, W>::Context Context;

    void read(Context* ctx, R msg)
    {
        ctx->fireRead(std::forward<R>(msg));//不做处理,直接转发
    }
    void write(Context* ctx, W msg)
    {
        ctx->fireWrite(std::forward<W>(msg));//不做处理,直接转发给下一个ctx处理
    }
};

} //libext
