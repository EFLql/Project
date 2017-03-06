#include <libext/ThreadPool/IOThreadPoolExecutor.h>

namespace libext
{
IOThreadPoolExecutor::IOThreadPoolExecutor(int numThreads)
: ThreadPoolExecutor(numThreads)
{
}

IOThreadPoolExecutor::~IOThreadPoolExecutor()
{
}

void IOThreadPoolExecutor::threadRun(ThreadPtr thread)
{
}

void IOThreadPoolExecutor::addObserver(std::shared_ptr<libext::ThreadPoolExecutor::Observer> o)
{
}

void IOThreadPoolExecutor::removeObserver(std::shared_ptr<libext::ThreadPoolExecutor::Observer> o)
{
}

} //libext
