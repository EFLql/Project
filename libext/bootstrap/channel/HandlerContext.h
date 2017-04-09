#pragma once
#include <memory>
#include <iostream>

namespace libext
{
class PipelineBase;

template <class In, class Out>
class HandlerContext
{
public:
    virtual ~HandlerContext() = default;

    virtual void fireRead(In msg) = 0;
    virtual void fireReadEOF() = 0;
    virtual void firetransportActive() = 0;
    virtual void firetransportInActive() = 0;
    virtual void fireWrite(Out msg) = 0;
    virtual void fireClose() = 0;

    virtual PipelineBase* getPipeline() = 0;
};

template <class In>
class InboundHandlerContext
{
public:
    virtual ~InboundHandlerContext() = default;

    virtual void fireRead(In msg) = 0;
    virtual void fireReadEOF() = 0;
    virtual void firetransportActive() = 0;
    virtual void firetransportInActive() = 0;

    virtual PipelineBase* getPipeline() = 0;
};

template <class Out>
class OutboundHandlerContext
{
public:
    virtual ~OutboundHandlerContext() = default;

    virtual void fireWrite(Out msg);
    virtual void fireClose();

    virtual PipelineBase* getPipeline() = 0;
};

#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

enum class HandlerDir
{
    IN,
    OUT,
    BOTH
};

} //libext

#include <libext/bootstrap/channel/HandlerContext-inl.h>