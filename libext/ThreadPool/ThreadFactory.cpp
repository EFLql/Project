#include "ThreadFactory.h"

using libext::ThreadFactory;

ThreadFactory::ThreadFactory()
{
}

ThreadFactory::~ThreadFactory()
{
}

std::thread ThreadFactory::newThread(libext::Func func)
{
	std::thread thread(func);
	return std::move(thread);
}
