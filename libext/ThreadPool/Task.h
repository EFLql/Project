#pragma once
#include <chrono>
#include "typedefine.h"
#include <memory>

namespace libext
{

class Task
{
public:
	Task(Func func, Func expireCallback, std::chrono::milliseconds expireTime) 
        : func_(func), expireCallback_(expireCallback), expireTime_(expireTime)
    {
        pendingTime_ = std::chrono::steady_clock::now();
    }

	virtual ~Task();

//private:
	Func func_;
    Func expireCallback_;

	std::chrono::steady_clock::time_point pendingTime_;
    std::chrono::steady_clock::time_point startTime_;
	std::chrono::steady_clock::time_point runTime_;
	std::chrono::milliseconds expireTime_;
};
typedef std::shared_ptr<Task> TaskPtr;


}//libext
