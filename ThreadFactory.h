/*	@file ThreadFactory
/*	@brief 创建线程工厂类
/*
/*	@author liqilin
/*	@date	2016/12/2

/*	@note
/*	@note
/*	
/*	@warning
/*
 */
#include "typedefine.h"

namespace libext
{

class ThreadFactory
{
public:
	virtual ~ThreadFactory();
	virtual _HANDLE newThread(libext::Func func) = 0;
};

class NamedThreadFactory : public ThreadFactory
{
public:
	explicit NamedThreadFactory(std::string name)
		:name_(name){}
	_HANDLE newThread(libext::Func func) override
	{
#ifdef unix
		_HANDLE hanle = std::thread(func);
		libext::setThreadName(hanle.native_handle(), name_);
#else
		_HANDLE handle = _beginthreadex(NULL, 0, func, NULL, 0, name_.c_str());
#endif
		return handle;
	}

private:
	std::string name_;
};

#ifdef unix
typedef  std::shared_ptr<ThreadFactory> ThreadFactoryPtr;
#else
typedef ThreadFactory* ThreadFactoryPtr;
#endif

}//namespace libext