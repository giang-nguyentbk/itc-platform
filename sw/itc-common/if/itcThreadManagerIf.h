#pragma once

#include <iostream>
#include <cstdint>
#include <pthread.h>
#include <vector>
#include <memory>

#include "itc.h"
#include "itcCWrapperIf.h"

// #include <enumUtils.h>

// using namespace CommonUtils::V1::EnumUtils;

namespace ItcPlatform
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

constexpr int32_t ITC_THREAD_SELF_LIMIT_PRIORITY_HIGH = 40;

struct Signal
{
	pthread_cond_t 			cond;
	pthread_condattr_t		condAttrs;
	pthread_mutex_t			mtx;
	pthread_mutexattr_t		mtxAttrs;
	struct timespec 		timeout;
	uint32_t				relativeTimeout {0};
	
	Signal(uint32_t clockMode = CLOCK_MONOTONIC)
	{
		init(clockMode);
	}
	virtual ~Signal()
	{
		reset();
	}
	
	Signal(const Signal &other) = default;
	Signal &operator=(const Signal &other) = default;
	Signal(Signal &&other) noexcept = default;
	Signal &operator=(Signal &&other) noexcept = default;

	void setTimeout(uint32_t iTimeout /* ms */)
	{
		relativeTimeout = iTimeout;
	}
	
	void calculateExpiredDate(uint32_t clockMode = CLOCK_MONOTONIC)
	{
		CWrapperIf::getInstance().lock()->cMemset(&timeout, 0, sizeof(struct timespec));
		CWrapperIf::getInstance().lock()->cClockGetTime(clockMode, &timeout);
		timeout.tv_sec 		+= relativeTimeout/ 1000;
		timeout.tv_nsec 	+= (relativeTimeout % 1000) * 1000000;
		if(timeout.tv_nsec >= 1000000000)
		{
			timeout.tv_sec		+= 1;
			timeout.tv_nsec	-= 1000000000;
		}
	}
	
private:
	void init(uint32_t clockMode)
	{
		auto cWrapperIf = CWrapperIf::getInstance().lock();
		if(!cWrapperIf)
		{
			return;
		}
		
		int32_t ret = cWrapperIf->cPthreadCondAttrInit(&condAttrs);
		if(ret != 0) UNLIKELY
		{
			TPT_TRACE(TRACE_ERROR, SSTR("Failed to pthread_condattr_init, error code = ", ret));
			return;
		}
		
		ret = cWrapperIf->cPthreadMutexAttrInit(&mtxAttrs);
		if(ret != 0) UNLIKELY
		{
			TPT_TRACE(TRACE_ERROR, SSTR("Failed to pthread_mutexattr_init, error code = ", ret));
			return;
		}
		ret = cWrapperIf->cPthreadCondAttrSetClock(&condAttrs, clockMode);
		if(ret != 0) UNLIKELY
		{
			TPT_TRACE(TRACE_ERROR, SSTR("Failed to pthread_condattr_setclock, error code = ", ret));
			return;
		}
		
		ret = cWrapperIf->cPthreadCondInit(&cond, &condAttrs);
		if(ret != 0) UNLIKELY
		{
			TPT_TRACE(TRACE_ERROR, SSTR("Failed to pthread_cond_init, error code = ", ret));
		}
		
		ret = cWrapperIf->cPthreadMutexInit(&mtx, &mtxAttrs);
		if(ret != 0) UNLIKELY
		{
			TPT_TRACE(TRACE_ERROR, SSTR("Failed to pthread_mutex_init, error code = ", ret));
		}
	}
	
	void reset()
	{
		auto cWrapperIf = CWrapperIf::getInstance().lock();
		if(!cWrapperIf)
		{
			return;
		}
		int32_t ret = cWrapperIf->cPthreadCondAttrDestroy(&condAttrs);
		if(ret != 0) UNLIKELY
		{
			TPT_TRACE(TRACE_ERROR, SSTR("Failed to pthread_condattr_destroy error code = ", ret));
		}
		
		ret = cWrapperIf->cPthreadMutexAttrDestroy(&mtxAttrs);
		if(ret != 0) UNLIKELY
		{
			TPT_TRACE(TRACE_ERROR, SSTR("Failed to pthread_mutexattr_destroy error code = ", ret));
		}
		
		ret = cWrapperIf->cPthreadCondDestroy(&cond);
		if(ret != 0) UNLIKELY
		{
			TPT_TRACE(TRACE_ERROR, SSTR("Failed to pthread_cond_destroy error code = ", ret));
		}
		
		ret = cWrapperIf->cPthreadMutexDestroy(&mtx);
		if(ret != 0) UNLIKELY
		{
			TPT_TRACE(TRACE_ERROR, SSTR("Failed to pthread_mutex_destroy error code = ", ret));
		}
	}
};

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
    bool                    					isRunning {false};
    bool                    					useHighestPriority {false};
    std::weak_ptr<Signal>						signal;
	
	Thread(const Task &iTask, bool iUseHighestPriority, std::shared_ptr<Signal> &iSignal)
		: task(iTask),
		  useHighestPriority(iUseHighestPriority),
		  signal(iSignal)
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
			signal = std::move(other.signal);
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
		signal.reset();
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
	
    virtual ThreadManagerIfReturnCode setThreadAttributes(int32_t policy, int32_t selfLimitPrio, int32_t priority) = 0;
    virtual ThreadManagerIfReturnCode addThread(const Task &task, bool useHighestPriority, std::shared_ptr<Signal> &signal) = 0;
    virtual ThreadManagerIfReturnCode startAllThreads() = 0;
    virtual ThreadManagerIfReturnCode terminateAllThreads() = 0;
    
protected:
    ThreadManagerIf() = default;
	
}; // class ThreadManagerIf

using ThreadManagerIfSharedPtr = std::shared_ptr<ThreadManagerIf>;

} // namespace INTERNAL
} // namespace ItcPlatform
