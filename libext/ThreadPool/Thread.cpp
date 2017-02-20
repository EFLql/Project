#include "Thread.h"

using libext::Thread;

Thread::~Thread()
{
}

void Thread::addTask(TaskWrapper task)
{
    taskQueue_.pushback( std::move(task) );
}

Thread::TaskWrapper Thread::getTask()
{
    auto task = taskQueue_.take();
    return std::move(task);
}
