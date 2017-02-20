#pragma once
#include <thread>
#include <chrono>
#include <functional>
#include "Task.h"
#include "Queue.h"
#include <atomic>

namespace libext
{

class Thread
{
public:
	Thread() : idle(false), shouldRunning(false), pendingTask(0) {}
	~Thread();

    typedef std::function<void (libext::TaskPtr)> TaskWrapper;

    void addTask(TaskWrapper task);
    TaskWrapper getTask();
	std::thread handler_;

    std::atomic<bool> idle;
    std::atomic<bool> shouldRunning;
    int pendingTask;
private:
    std::mutex mutex_;
    Queue<TaskWrapper> taskQueue_;
};
typedef std::shared_ptr<Thread> ThreadPtr;


}//libext
