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

void IOThreadPoolExecutor::addTask(libext::Func fun, libext::Func expireCallback)
{
    auto thread = pickThread();
}

//schedule thread
libext::ThreadPtr ThreadPoolExecutor::pickThread()
{
    static int s = 0;
    s = (s + 1) % vectThreads_.size();
    return vectThreads_[s];
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
