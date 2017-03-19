#pragma once
#include <thread>
#include "typedefine.h"

namespace libext
{

class ThreadFactory
{
public:
	ThreadFactory();
	virtual ~ThreadFactory();
	
public:
	std::thread newThread(libext::Func&& func);
};


}//libext
