#pragma once
#include <memory>

namespace libext
{
class AsyncTransport
{
public:
    virtual ~AsyncTransport() = default;
    typedef std::unique_ptr<AsyncTransport> UniquePtr;
};

}
