#pragma once
#include <stddef.h>
#include <functional>
namespace libext
{

typedef char int8_t, byte_t;
typedef unsigned char uint8_t, ubyte_t;
typedef short int16_t, WORD;
typedef unsigned short uint16_t;
typedef int int32_t, DWORD;
typedef unsigned int uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;
typedef size_t ssize_t;

typedef std::function<void()> Func;
}
