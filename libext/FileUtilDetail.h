#pragma once
#include <errno.h>
#include <libext/typedef.h>
namespace libext
{
template <class F, class... Args>
ssize_t wrapNoInt(F f, Args... args)
{
    ssize_t ret = 0;
    do
    {
        ret = f(args...);
    }while(ret == -1 && errno == EINTR);
    return ret;
}

} //libext
