#pragma once

#include "itcThreadManagerIf.h"

#include <iostream>
#include <cstdint>
#include <pthread.h>
#include <vector>
#include <memory>
#include <gtest/gtest.h>

#include "itc.h"
#include "itcCWrapperIf.h"
#include "itcConstant.h"

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

class ThreadManager : public ThreadManagerIf
{
public:
	static std::weak_ptr<ThreadManager> getInstance();
    virtual ~ThreadManager()
	{
		terminateAllThreads();
		reset();
	}
	
	ThreadManager(const ThreadManager &other) = delete;
	ThreadManager &operator=(const ThreadManager &other) = delete;
	ThreadManager(ThreadManager &&other) noexcept = delete;
	ThreadManager &operator=(ThreadManager &&other) noexcept = delete;
	
    ThreadManagerIfReturnCode setThreadAttributes(int32_t policy = SCHED_OTHER, int32_t selfLimitPrio = ITC_THREAD_SELF_LIMIT_PRIORITY_HIGH, int32_t priority = 0) override;
    ThreadManagerIfReturnCode addThread(const Task &task, std::shared_ptr<SyncObject> syncObj, bool useHighestPriority = false) override;
    ThreadManagerIfReturnCode startAllThreads() override;
    ThreadManagerIfReturnCode terminateAllThreads() override;

private:
	ThreadManager()
	{
		CWrapperIf::getInstance().lock()->cPthreadMutexInit(&m_threadListMutex, nullptr);
		m_threadList.reserve(8);
	}
    
private:
	void reset();
	ThreadManagerIfReturnCode checkSchedulingParams(int32_t policy, int32_t selfLimitPrio, int32_t priority);
	ThreadManagerIfReturnCode configureAndStartThread(void *(*taskFunc)(void *), void *args, pthread_t *tid, int policy, int priority);

private:
	SINGLETON_DECLARATION(ThreadManager)
	
    pthread_mutex_t 				m_threadListMutex;
    std::vector<Thread>   			m_threadList;
    int32_t 						m_policy {SCHED_OTHER};
    int32_t 						m_selfLimitPrio {0};
    int32_t 						m_priority {0};
	
	friend class ThreadManagerIfTest;
	FRIEND_TEST(ThreadManagerIfTest, checkSchedulingParamsTest1);
	FRIEND_TEST(ThreadManagerIfTest, checkSchedulingParamsTest2);
	FRIEND_TEST(ThreadManagerIfTest, checkSchedulingParamsTest3);
	FRIEND_TEST(ThreadManagerIfTest, checkSchedulingParamsTest4);
	FRIEND_TEST(ThreadManagerIfTest, configureAndStartThreadTest1);
	FRIEND_TEST(ThreadManagerIfTest, configureAndStartThreadTest2);
	FRIEND_TEST(ThreadManagerIfTest, configureAndStartThreadTest3);
	FRIEND_TEST(ThreadManagerIfTest, setThreadAttributesTest1);
	FRIEND_TEST(ThreadManagerIfTest, setThreadAttributesTest2);
	FRIEND_TEST(ThreadManagerIfTest, setThreadAttributesTest3);
	FRIEND_TEST(ThreadManagerIfTest, addThreadTest1);
	FRIEND_TEST(ThreadManagerIfTest, startAllThreadsTest1);
	FRIEND_TEST(ThreadManagerIfTest, startAllThreadsTest2);
	FRIEND_TEST(ThreadManagerIfTest, startAllThreadsTest3);
	FRIEND_TEST(ThreadManagerIfTest, terminateAllThreadsTest1);
}; // class ThreadManager

using ThreadManagerSharedPtr = std::shared_ptr<ThreadManager>;

} // namespace INTERNAL
} // namespace ITC
