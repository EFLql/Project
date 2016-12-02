#pragma once

namespace libext
{

#ifdef unix
	typedef std::function<void()> Func;
	typedef std::thread _HANDLE;
#else
	typedef void (*winFun)();
	typedef winFun Func;
	typedef HANDLE _HANDLE;
#endif

}//namespace libext


/*int*/
typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
/*byte*/
typedef char byte_t;
typedef unsigned char ubyte_t;
/*word*/
typedef short word_t;
typedef unsigned short uword_t;
/*dword*/
typedef int dword_t;
typedef unsigned int udword_t;