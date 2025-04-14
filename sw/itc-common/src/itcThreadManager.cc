#include "itcThreadManager.h"

// #include <traceIf.h>
// #include "itc-common/inc/itcTptProvider.h"

#include "itc.h"
#include "itcMutex.h"
#include "itcCWrapperIf.h"

namespace ITC
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{
// using namespace CommonUtils::V1::EnumUtils;
using namespace ITC::PROVIDED;
using ThreadManagerIfReturnCode = ThreadManager::ThreadManagerIfReturnCode;

/***
 * Just for compilation in unit testing.
 * Those getInstance from <InterfaceClassName>If interface classes
 * must look like this to avoid multiple definition compiler error
 * which is conflicts between this real getInstance implementation
 * and the one is in mock version.
 * 
 * For example, find itcThreadManagerIfMock.cc to see mocked implementation
 * of getInstance().
*/
#ifndef UNITTEST
SINGLETON_IF_DEFINITION(ThreadManagerIf, ThreadManager)
#endif

SINGLETON_DEFINITION(ThreadManager)

void ThreadManager::reset()
{
	MUTEX_LOCK(&m_threadListMutex);
	m_threadList.clear();
	MUTEX_UNLOCK(&m_threadListMutex);
	CWrapperIf::getInstance().lock()->cPthreadMutexDestroy(&m_threadListMutex);
	m_policy = SCHED_OTHER;
	m_selfLimitPrio = 0;
	m_priority = 0;
}

ThreadManagerIfReturnCode ThreadManager::setThreadAttributes(int32_t policy, int32_t selfLimitPrio, int32_t priority)
{
    if(policy == SCHED_OTHER)
	{
		TPT_TRACE(TRACE_INFO, SSTR("Configure SCHED_OTHER for this thread!"));
		m_policy = SCHED_OTHER;
		m_selfLimitPrio = CWrapperIf::getInstance().lock()->cSchedGetPriorityMax(SCHED_OTHER);
		m_priority = CWrapperIf::getInstance().lock()->cSchedGetPriorityMin(SCHED_OTHER);
	} else
	{
		
		auto rc = checkSchedulingParams(policy, selfLimitPrio, priority);
		if(rc != MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_OK))
		{
			TPT_TRACE(TRACE_ABN, SSTR("Failed to checkSchedulingParams()!"));
			return rc;
		}

		m_policy = policy;
		m_selfLimitPrio = selfLimitPrio;
		m_priority = (priority > selfLimitPrio) ? selfLimitPrio : priority;
	}
	return MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_OK);
}

ThreadManagerIfReturnCode ThreadManager::addThread(const Task &task, std::shared_ptr<SyncObject> syncObj, bool useHighestPriority)
{
	MUTEX_LOCK(&m_threadListMutex);
	m_threadList.emplace_back(task, syncObj, useHighestPriority);
	MUTEX_UNLOCK(&m_threadListMutex);
	return MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_OK);
}

ThreadManagerIfReturnCode ThreadManager::startAllThreads()
{
	auto rc = MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_OK);
	MUTEX_LOCK(&m_threadListMutex);
	for(auto &thr : m_threadList)
	{
		if(thr.isRunning) UNLIKELY continue;
		
		auto syncObj = thr.syncObj.lock();
		if(!syncObj) UNLIKELY
		{
			TPT_TRACE(TRACE_ERROR, SSTR("The owner of the thread tid = ", thr.tid, " is going to release resources!"));
			rc = MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_SYSCALL_ERROR);
			break;
		}
		
		MUTEX_LOCK(&syncObj->elems->mtx);
		rc = configureAndStartThread(thr.task.taskFunc, thr.task.taskArgs, &thr.tid, m_policy,
					(thr.useHighestPriority ? m_selfLimitPrio : m_priority));
		if(rc != MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_OK)) UNLIKELY
		{
			TPT_TRACE(TRACE_ERROR, SSTR("Failed to start a thread tid = ", thr.tid));
			break;
		}
		
		syncObj->calculateExpiredDate();
		int32_t ret = CWrapperIf::getInstance().lock()->cPthreadCondTimedWait(&syncObj->elems->cond, &syncObj->elems->mtx, &syncObj->timeout);
		MUTEX_UNLOCK(&syncObj->elems->mtx);
		if(ret != 0) UNLIKELY
		{
			TPT_TRACE(TRACE_INFO, SSTR("Failed to start thread tid = ", thr.tid, ", waitTime = ", \
				syncObj->relativeTimeout, " (ms), ret = ", ret));
			rc = MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_INITIALISATION_TIMEOUT);
			break;
		}
		TPT_TRACE(TRACE_INFO, SSTR("Started a thread, tid = ", thr.tid));
		thr.isRunning = true;
	}
	MUTEX_UNLOCK(&m_threadListMutex);
	return rc;
}

ThreadManagerIfReturnCode ThreadManager::terminateAllThreads()
{
	auto rc = MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_OK);
	MUTEX_LOCK(&m_threadListMutex);
	for(auto &thr : m_threadList)
	{
		if(!thr.isRunning) UNLIKELY continue;
		
		int ret = CWrapperIf::getInstance().lock()->cPthreadCancel(thr.tid);
		if(ret != 0) UNLIKELY
		{
			TPT_TRACE(TRACE_ERROR, SSTR("Failed to pthread_cancel, error code = ", ret));
			rc = ThreadManagerIfReturnCode::THREAD_MANAGER_SYSCALL_ERROR;
			break;
		}

		ret = CWrapperIf::getInstance().lock()->cPthreadJoin(thr.tid, nullptr);
		if(ret != 0) UNLIKELY
		{
			TPT_TRACE(TRACE_ABN, SSTR("Failed to pthread_join, error code = ", ret));
			rc = ThreadManagerIfReturnCode::THREAD_MANAGER_SYSCALL_ERROR;
			break;
		}

		TPT_TRACE(TRACE_INFO, SSTR("Terminating a thread, tid = ", thr.tid));
		thr.isRunning = false;
	}
	MUTEX_UNLOCK(&m_threadListMutex);
	return rc;
}

ThreadManagerIfReturnCode ThreadManager::checkSchedulingParams(int32_t policy, int32_t selfLimitPrio, int32_t priority)
{
    int maxPrio = CWrapperIf::getInstance().lock()->cSchedGetPriorityMax(policy);
	int minPrio = CWrapperIf::getInstance().lock()->cSchedGetPriorityMin(policy);
	if(priority < minPrio || priority > maxPrio || selfLimitPrio < minPrio || selfLimitPrio > maxPrio) UNLIKELY
	{
		TPT_TRACE(TRACE_ABN, SSTR("Invalid priority config, prio = ", priority, ", minPrio = ", minPrio, ", maxPrio = ", maxPrio, ", selfLimitPrio = ", selfLimitPrio, "!"));
		return MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_INVALID_ARGUMENTS);
	}
    return MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_OK);
}

ThreadManagerIfReturnCode ThreadManager::configureAndStartThread(void *(*taskFunc)(void *), void *args, pthread_t *tid, int policy, int priority)
{
	auto cWrapperIf = CWrapperIf::getInstance().lock();
	if(!cWrapperIf)
	{
		TPT_TRACE(TRACE_ERROR, SSTR("CWrapperIf was not initialised!"));
		return MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_SYSCALL_ERROR);
	}
	
	pthread_attr_t threadAttrs;
	int ret = cWrapperIf->cPthreadAttrInit(&threadAttrs);
	if(ret != 0) UNLIKELY
	{
		TPT_TRACE(TRACE_ERROR, SSTR("Failed to pthread_attr_init, error code = ", ret));
		return MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_SYSCALL_ERROR);
	}
	ret = cWrapperIf->cPthreadAttrSetSchedPolicy(&threadAttrs, policy);
	if(ret != 0) UNLIKELY
	{
		TPT_TRACE(TRACE_ERROR, SSTR("Failed to pthread_attr_setschedpolicy, error code = ", ret));
		return MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_SYSCALL_ERROR);
	}
	struct sched_param schedParams;
	schedParams.sched_priority = priority;
	ret = cWrapperIf->cPthreadAttrSetSchedParam(&threadAttrs, &schedParams);
	if(ret != 0) UNLIKELY
	{
		TPT_TRACE(TRACE_ERROR, SSTR("Failed to pthread_attr_setschedparam, error code = ", ret));
		return MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_SYSCALL_ERROR);
	}
	ret = cWrapperIf->cPthreadAttrSetInheritSched(&threadAttrs, PTHREAD_EXPLICIT_SCHED);
	if(ret != 0) UNLIKELY
	{
		TPT_TRACE(TRACE_ERROR, SSTR("Failed to pthread_attr_setinheritsched, error code = ", ret));
		return MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_SYSCALL_ERROR);
	}
	
	ret = cWrapperIf->cPthreadCreate(tid, &threadAttrs, taskFunc, args);
	if(ret != 0) UNLIKELY
	{
		TPT_TRACE(TRACE_ERROR, SSTR("Failed to pthread_create, error code = ", ret));
		return MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_SYSCALL_ERROR);
	}
	return MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_OK);
}

} // namespace INTERNAL
} // namespace ITC