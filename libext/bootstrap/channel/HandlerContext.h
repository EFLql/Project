#pragma once
#include <libext/typedef.h>
#include <memory>
#include <iostream>
#include <utility>

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
    virtual void fireTransportActive() = 0;
    virtual void fireTransportInActive() = 0;
    virtual void fireWrite(Out msg) = 0;
    virtual void fireClose() = 0;

    virtual PipelineBase* getPipeline() = 0;
    virtual void setReadBufferSetting(
            uint64_t minAvailable,
            uint64_t allocationSize) = 0;
    virtual std::pair<uint64_t, uint64_t> 
        getReadBufferSetting() = 0;
};

template <class In>
class InboundHandlerContext
{
public:
    virtual ~InboundHandlerContext() = default;

    virtual void fireRead(In msg) = 0;
    virtual void fireReadEOF() = 0;
    virtual void fireTransportActive() = 0;
    virtual void fireTransportInActive() = 0;

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
