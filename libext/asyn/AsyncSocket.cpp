#include <libext/asyn/AsyncSocket.h>

namespace libext
{
AsyncSocket::AsyncSocket(EventBase* evb, int fd)
: maxReadsPerEvent_(16)
{
}

AsyncSocket::~AsyncSocket()
{
}

} //libext
