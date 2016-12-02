#pragma once

#include<thread>
#ifdef unix
#include <pthread.h>
#endif

namespace libext
{
	
#if defined(__GLIBC__) && !defined(__APPLE__) && !defined(__ANDROID__)
#if __GLIBC_PREREQ(2, 12)
// has pthread_setname_np(pthread_t, const char*) (2 params)
#define LIBEXT_HAS_PTHREAD_SETNAME_NP_THREAD_NAME 1
#endif
#endif

#if defined(__APPLE__) && defined(__MAC_OS_X_VERSION_MIN_REQUIED)
#if __MAC_OS_X_VERSION_MIN_REQUIED >= 1060
// has pthread_setname_np(const char*) (1 params)
#define LIBEXT_HAS_PTHREAD_SETNAME_NP_NAME 1
#endif
#endif

#ifdef LIBEXT_HAS_PTHREAD_SETNAME_NP_THREAD_NAME
template<>
inline bool setThreadName(pthread_t id, std::string name)
{
	std::string prefix = name.substr(0, 15);//Linux��pthread_setname_np�߳������ֻ��16���ַ�
	return pthread_setname_np(id, prefix.c_str());
}
#endif

#ifdef LIBEXT_HAS_PTHREAD_SETNAME_NP_NAME\
template<>
inline bool setThreadName(pthread_t id, std::string name)
{
	if(pthread_equal(pthread_self(), id))//��Щ����ϵͳֻ�����ñ��̵߳��߳����ƶ��޷��ı������̵߳�����
	{
		std::string prefix = name.substr(0, 15);
		return pthread_setname_np(prefix.c_str());
	}
	return false;
}
#endif

#ifdef unix
inline bool setThreadName(std::string name)
{
	return setThreadName(pthread_self(), name);
}
#else
inline bool setThreadName(std::string name)
{
	return false;
}
#endif



}