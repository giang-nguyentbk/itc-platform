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
	
    ThreadManagerIfReturnCode setThreadAttributes(int32_t policy, int32_t selfLimitPrio, int32_t priority) override;
    ThreadManagerIfReturnCode addThread(const Task &task, bool useHighestPriority, std::shared_ptr<Signal> &signal) override;
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
	static std::shared_ptr<ThreadManager> m_instance;
	static std::mutex m_singletonMutex;
    pthread_mutex_t 				m_threadListMutex;
    std::vector<Thread>   			m_threadList;
    int32_t 						m_policy {SCHED_OTHER};
    int32_t 						m_selfLimitPrio {0};
    int32_t 						m_priority {0};
	
	friend class ThreadManagerIfTest;
	FRIEND_TEST(ThreadManagerIfTest, checkSchedulingParamsTestCase1);
	FRIEND_TEST(ThreadManagerIfTest, checkSchedulingParamsTestCase2);
	FRIEND_TEST(ThreadManagerIfTest, checkSchedulingParamsTestCase3);
	FRIEND_TEST(ThreadManagerIfTest, checkSchedulingParamsTestCase4);
	FRIEND_TEST(ThreadManagerIfTest, configureAndStartThreadTestCase1);
	FRIEND_TEST(ThreadManagerIfTest, configureAndStartThreadTestCase2);
	FRIEND_TEST(ThreadManagerIfTest, configureAndStartThreadTestCase3);
	FRIEND_TEST(ThreadManagerIfTest, setThreadAttributesTestCase1);
	FRIEND_TEST(ThreadManagerIfTest, setThreadAttributesTestCase2);
	FRIEND_TEST(ThreadManagerIfTest, setThreadAttributesTestCase3);
	FRIEND_TEST(ThreadManagerIfTest, addThreadTestCase1);
	FRIEND_TEST(ThreadManagerIfTest, startAllThreadsTestCase1);
	FRIEND_TEST(ThreadManagerIfTest, startAllThreadsTestCase2);
	FRIEND_TEST(ThreadManagerIfTest, startAllThreadsTestCase3);
	FRIEND_TEST(ThreadManagerIfTest, terminateAllThreadsTestCase1);
}; // class ThreadManager

using ThreadManagerSharedPtr = std::shared_ptr<ThreadManager>;

} // namespace INTERNAL
} // namespace ItcPlatform
