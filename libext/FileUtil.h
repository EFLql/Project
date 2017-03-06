#pragma once
#include <libext/typedef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
namespace libext
{

ssize_t openNoInt(const char* name, int flag, uint16_t mode);
ssize_t closeNoInt(int fd);
ssize_t readNoInt(int fd, void* buf, size_t n);
ssize_t writeNoInt(int fd, const char* buf, size_t n);

} //libext
