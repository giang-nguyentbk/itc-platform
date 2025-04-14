#pragma once

#include <iostream>
#include <cstdint>
#include <vector>
#include <memory>
#include <functional>

#include <pthread.h>
#include "itc.h"
#include "itcCWrapperIf.h"
#include "itcSyncObject.h"

// #include <enumUtils.h>

// using namespace CommonUtils::V1::EnumUtils;

namespace ITC
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

#define ITC_THREAD_SELF_LIMIT_PRIORITY_HIGH 	(int32_t)(40)

struct Task
{
	void                    			*(*taskFunc)(void *);
    void     							*taskArgs {nullptr};
	Task(void *(*iTaskFunc)(void *), void *itaskArgs = nullptr)
		: taskFunc(iTaskFunc),
		  taskArgs(itaskArgs)
	{}
	
	Task() = default;
	virtual ~Task() = default;
	
	Task(const Task &other) = default;
	Task &operator=(const Task &other) = default;
	Task(Task &&other) noexcept = default;
	Task &operator=(Task &&other) noexcept = default;
};

struct Thread
{
    pthread_t									tid {0};
    Task										task;
    std::weak_ptr<SyncObject>					syncObj;
    bool                    					isRunning {false};
    bool                    					useHighestPriority {false};
	
	Thread(const Task &iTask, std::shared_ptr<SyncObject> iSyncObj, bool iUseHighestPriority = false)
		: task(iTask),
		  syncObj(iSyncObj),
		  useHighestPriority(iUseHighestPriority)
	{}
	virtual ~Thread()
	{
		reset();
	}
	
	Thread(const Thread &other) = delete;
	Thread &operator=(const Thread &other) = delete;
	Thread(Thread &&other) noexcept
	{
		if(this != &other)
		{
			tid = std::move(other.tid);
			task = std::move(other.task);
			isRunning = std::move(other.isRunning);
			useHighestPriority = std::move(other.useHighestPriority);
			syncObj = std::move(other.syncObj);
			other.reset();
		}
	}
	Thread &operator=(Thread &&other) noexcept
	{
		if(this != &other)
		{
			reset();
			*this = std::move(other);
		}
		
		return *this;
	}
	
private:
	void reset()
	{
		if(tid)
		{
			CWrapperIf::getInstance().lock()->cPthreadCancel(tid);
			CWrapperIf::getInstance().lock()->cPthreadJoin(tid, nullptr);
		}
		task.taskFunc = nullptr;
		task.taskArgs = nullptr;
		isRunning = false;
		useHighestPriority = false;
		syncObj.reset();
	}
};

class ThreadManagerIf
{
public:
    static std::weak_ptr<ThreadManagerIf> getInstance();
    virtual ~ThreadManagerIf() = default;

	ThreadManagerIf(const ThreadManagerIf &other) = delete;
	ThreadManagerIf &operator=(const ThreadManagerIf &other) = delete;
	ThreadManagerIf(ThreadManagerIf &&other) noexcept = delete;
	ThreadManagerIf &operator=(ThreadManagerIf &&other) noexcept = delete;
    
	enum class ThreadManagerIfReturnCode
	{
		THREAD_MANAGER_UNDEFINED,
		THREAD_MANAGER_OK,
		THREAD_MANAGER_SYSCALL_ERROR,
		THREAD_MANAGER_INVALID_ARGUMENTS,
		THREAD_MANAGER_INITIALISATION_TIMEOUT,
	};
	
	// enum class ThreadManagerIfReturnCodeRaw
	// {
	// 	THREAD_MANAGER_UNDEFINED,
	// 	THREAD_MANAGER_OK,
	// 	THREAD_MANAGER_SYSCALL_ERROR,
	// 	THREAD_MANAGER_INVALID_ARGUMENTS,
	// };

	// class ThreadManagerIfReturnCode : public EnumType<ThreadManagerIfReturnCodeRaw>
	// {
	// public:
	// 	explicit ThreadManagerIfReturnCode(const ThreadManagerIfReturnCodeRaw& raw) : EnumType<ThreadManagerIfReturnCodeRaw>(raw) {}
	// 	explicit ThreadManagerIfReturnCode() : EnumType<ThreadManagerIfReturnCodeRaw>(ThreadManagerIfReturnCodeRaw::THREAD_MANAGER_UNDEFINED) {}

	// 	std::string toString() const override
	// 	{
	// 		switch (getRawEnum())
	// 		{
	// 		case ThreadManagerIfReturnCodeRaw::THREAD_MANAGER_UNDEFINED:
	// 			return "THREAD_MANAGER_UNDEFINED";

	// 		case ThreadManagerIfReturnCodeRaw::THREAD_MANAGER_OK:
	// 			return "THREAD_MANAGER_OK";

	// 		case ThreadManagerIfReturnCodeRaw::THREAD_MANAGER_SYSCALL_ERROR:
	// 			return "THREAD_MANAGER_SYSCALL_ERROR";

	// 		case ThreadManagerIfReturnCodeRaw::THREAD_MANAGER_INVALID_ARGUMENTS:
	// 			return "THREAD_MANAGER_INVALID_ARGUMENTS";

	// 		default:
	// 			return "Unknown EnumType: " + std::to_string(toS32());
	// 		}
	// 	}
	// };
	
    virtual ThreadManagerIfReturnCode setThreadAttributes(int32_t policy = SCHED_OTHER, int32_t selfLimitPrio = ITC_THREAD_SELF_LIMIT_PRIORITY_HIGH, int32_t priority = 0) = 0;
    virtual ThreadManagerIfReturnCode addThread(const Task &task, std::shared_ptr<SyncObject> syncObj, bool useHighestPriority = false) = 0;
    virtual ThreadManagerIfReturnCode startAllThreads() = 0;
    virtual ThreadManagerIfReturnCode terminateAllThreads() = 0;
    
protected:
    ThreadManagerIf() = default;
	
}; // class ThreadManagerIf

using ThreadManagerIfSharedPtr = std::shared_ptr<ThreadManagerIf>;

} // namespace INTERNAL
} // namespace ITC
