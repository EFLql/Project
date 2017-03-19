#pragma once

#if defined(__clang__) || defined(__GNUC__) 
#define LIBEXT_ALIGNED(size) __attribute__((__aligned__(size)))
#elif defined(_MSC_VER)
#define LIBEXT_ALIGNED(size) __declspec(aligne(size))
#else
# error Cannot define LIBEXT_ALIGNED on this platform
#endif

//为了防止小的变量共享同一个cache line 导致在多线程环境下
//造成性能下降，这里将对齐值设置为128
//因为一般CPU的cache line的大小都不会超过128，所以这里将对其值
//设置为128,写demo测试过如果用默认对齐值性能可能会降低10倍!!
#define LIBEXT_ALIGNED_AVOID_FALSE_SHARED LIBEXT_ALIGNED(128)
