#include "ThreadFactory.h"

using libext::ThreadFactory;

ThreadFactory::ThreadFactory()
{
}

ThreadFactory::~ThreadFactory()
{
}

std::thread ThreadFactory::newThread(libext::Func&& func)
{
	auto thread = std::thread(std::move(func));
	return thread;
}
