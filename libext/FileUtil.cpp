#include <libext/FileUtil.h>
#include <libext/FileUtilDetail.h>
namespace libext
{
ssize_t openNoInt(const char* name, int flag, uint16_t mode)
{
   return wrapNoInt(open, name, flag, mode);
}

ssize_t closeNoInt(int fd)
{
    return wrapNoInt(close, fd);
}

ssize_t readNoInt(int fd, void* buf, size_t n)
{
    return wrapNoInt(read, fd, buf, n);
}

ssize_t writeNoInt(int fd, const char* buf, size_t n)
{
    return wrapNoInt(write, fd, buf, n);
}

} //libext
