#include "itc.h"
#include "itcThreadManager.h"
#include "itcMutex.h"

#include <iostream>
#include <memory>
#include <string>

#include <sched.h>
#include <gtest/gtest.h>

#include "itcCWrapperIfMock.h"

using namespace ItcPlatform::INTERNAL;
using namespace ItcPlatform::PROVIDED;
using ThreadManagerIfReturnCode = ThreadManager::ThreadManagerIfReturnCode;

namespace ItcPlatform
{
namespace INTERNAL
{

class ThreadManagerIfTest : public testing::Test
{
protected:
    ThreadManagerIfTest()
    {}

    ~ThreadManagerIfTest()
    {}
    
    void SetUp() override
    {
        m_threadManager = ThreadManager::getInstance().lock();
        m_cWrapperIfMock = CWrapperIfMock::getInstance().lock();
    }

    void TearDown() override
    {
        m_threadManager->m_instance.reset();
        m_cWrapperIfMock->m_instance.reset();
    }
    
    

protected:
    std::shared_ptr<ThreadManager> m_threadManager;
    std::shared_ptr<CWrapperIfMock> m_cWrapperIfMock;
    
};

TEST_F(ThreadManagerIfTest, checkSchedulingParamsTestCase1)
{
    /***
     * Test scenario: test set priority that exceeds max supported priority for policy SCHED_FIFO (1-99).
     */
    int32_t policy = SCHED_FIFO;
    int32_t selfLimitPrio = ITC_THREAD_SELF_LIMIT_PRIORITY_HIGH;
    int32_t invalidPriority = 100; /* NOK */
    auto rc = m_threadManager->checkSchedulingParams(policy, selfLimitPrio, invalidPriority);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_INVALID_ARGUMENTS));
}

TEST_F(ThreadManagerIfTest, checkSchedulingParamsTestCase2)
{
    /***
     * Test scenario: test set a valid priority within the permitted range (1-99).
     */
    int32_t policy = SCHED_FIFO;
    int32_t selfLimitPrio = ITC_THREAD_SELF_LIMIT_PRIORITY_HIGH;
    int32_t invalidPriority = 15; /* OK */
    auto rc = m_threadManager->checkSchedulingParams(policy, selfLimitPrio, invalidPriority);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_OK));
}

TEST_F(ThreadManagerIfTest, checkSchedulingParamsTestCase3)
{
    /***
     * Test scenario: test set a valid priority within the permitted range (1-99).
     */
    int32_t policy = SCHED_RR;
    int32_t selfLimitPrio = ITC_THREAD_SELF_LIMIT_PRIORITY_HIGH;
    int32_t invalidPriority = 15; /* OK */
    auto rc = m_threadManager->checkSchedulingParams(policy, selfLimitPrio, invalidPriority);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_OK));
}

TEST_F(ThreadManagerIfTest, checkSchedulingParamsTestCase4)
{
    /***
     * Test scenario: test set an invalid priority exceeding the permitted range (0-0).
     */
    int32_t policy = SCHED_OTHER;
    int32_t selfLimitPrio = ITC_THREAD_SELF_LIMIT_PRIORITY_HIGH;
    int32_t invalidPriority = 10; /* NOK */
    auto rc = m_threadManager->checkSchedulingParams(policy, selfLimitPrio, invalidPriority);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_INVALID_ARGUMENTS));
}

#define EXPECTED_KEY_VALUE 999
struct TaskFuncArgs
{
    int32_t key {0};
    pthread_cond_t *cond {nullptr};
    pthread_mutex_t *mtx {nullptr};
    bool forceInitTimeout {false};
    bool forcePause {false};
    uint32_t timeout {0};
};

void *taskFunc(void *arg)
{
    TaskFuncArgs *ptr = reinterpret_cast<TaskFuncArgs *>(arg);
    if(ptr)
    {
        if(ptr->forceInitTimeout)
        {
            /* Used to test thread initialisation timeout case. */
            sleep(10);
        }
        if(ptr->cond && ptr->mtx)
        {
            MUTEX_LOCK(ptr->mtx);
            pthread_cond_signal(ptr->cond);
            MUTEX_UNLOCK(ptr->mtx);
        }
        if(ptr->forcePause)
        {
            /* Used to test pthread_cancel case (sleep creates cancelation point). */
            sleep(ptr->timeout);
        }
        ptr->key = EXPECTED_KEY_VALUE;
    }
    return nullptr;
}

TEST_F(ThreadManagerIfTest, configureAndStartThreadTestCase1)
{
    /***
     * Test scenario: test start a thread with an invalid priority exceeding the permitted range (0-0).
     */
    int32_t policy = SCHED_OTHER;
    int32_t invalidPriority = 10;
    TaskFuncArgs args {0, nullptr, nullptr, false, false, 0};
    pthread_t tid;
    auto rc = m_threadManager->configureAndStartThread(taskFunc, &args, &tid, policy, invalidPriority);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_SYSCALL_ERROR));
}

TEST_F(ThreadManagerIfTest, configureAndStartThreadTestCase2)
{
    /***
     * Test scenario: test happy case that starts a thread and check the change of key to EXPECTED_KEY_VALUE.
     */
    int32_t policy = SCHED_OTHER;
    int32_t priority = 0;
    TaskFuncArgs args {0, nullptr, nullptr, false, false, 0};
    pthread_t tid;
    auto rc = m_threadManager->configureAndStartThread(taskFunc, &args, &tid, policy, priority);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_OK));
    usleep(5000); /* Small sleep 5ms to make sure to have keys set to EXPECTED_KEY_VALUE. */
    ASSERT_EQ(args.key, EXPECTED_KEY_VALUE);
    m_cWrapperIfMock->cPthreadCancel(tid);
    m_cWrapperIfMock->cPthreadDetach(tid);
}

TEST_F(ThreadManagerIfTest, configureAndStartThreadTestCase3)
{
    /***
     * Test scenario: test failed to start a Realtime thread without CAP_SYS_RESOURCE privilege.
     */
    int32_t policy = SCHED_RR;
    int32_t priority = 30;
    TaskFuncArgs args {0, nullptr, nullptr, false, false, 0};
    pthread_t tid;
    auto rc = m_threadManager->configureAndStartThread(taskFunc, &args, &tid, policy, priority);
    /***
     * The call to pthread_create will return EPERM,
     * research on CAP_SYS_RESOURCE, setrlimit, ulimit -r unlimited, RLIMIT_RTPRIO for more details.
     */
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_SYSCALL_ERROR));
}

TEST_F(ThreadManagerIfTest, setThreadAttributesTestCase1)
{
    /***
     * Test scenario: test set thread attributes corresponding to policy SCHED_OTHER.
     */
    int32_t selfLimitPrio = sched_get_priority_max(SCHED_OTHER);
    int32_t priority = sched_get_priority_min(SCHED_OTHER);
    auto rc = m_threadManager->setThreadAttributes(SCHED_OTHER, ITC_THREAD_SELF_LIMIT_PRIORITY_HIGH, 0);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_OK));
    ASSERT_EQ(m_threadManager->m_policy, SCHED_OTHER);
    ASSERT_EQ(m_threadManager->m_selfLimitPrio, selfLimitPrio);
    ASSERT_EQ(m_threadManager->m_priority, priority);
}

TEST_F(ThreadManagerIfTest, setThreadAttributesTestCase2)
{
    /***
     * Test scenario: test set thread attributes corresponding to policy SCHED_RR,
     * and priority is lower than self-limit ceiling priority.
     */
    int32_t selfLimitPrio = ITC_THREAD_SELF_LIMIT_PRIORITY_HIGH;
    int32_t priority = ITC_THREAD_SELF_LIMIT_PRIORITY_HIGH - 10;
    auto rc = m_threadManager->setThreadAttributes(SCHED_RR, selfLimitPrio, priority);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_OK));
    ASSERT_EQ(m_threadManager->m_policy, SCHED_RR);
    ASSERT_EQ(m_threadManager->m_selfLimitPrio, selfLimitPrio);
    ASSERT_EQ(m_threadManager->m_priority, priority);
}

TEST_F(ThreadManagerIfTest, setThreadAttributesTestCase3)
{
    /***
     * Test scenario: test set thread attributes corresponding to policy SCHED_RR,
     * and priority is limited at self-limit ceiling priority which is ITC_THREAD_SELF_LIMIT_PRIORITY_HIGH.
     */
    int32_t selfLimitPrio = ITC_THREAD_SELF_LIMIT_PRIORITY_HIGH;
    int32_t priority = ITC_THREAD_SELF_LIMIT_PRIORITY_HIGH + 61;
    auto rc = m_threadManager->setThreadAttributes(SCHED_RR, selfLimitPrio, priority);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_INVALID_ARGUMENTS));
}

TEST_F(ThreadManagerIfTest, addThreadTestCase1)
{
    /***
     * Test scenario: test set add a thread into m_threadList.
     */
    auto signal = std::make_shared<Signal>();
    auto rc = m_threadManager->addThread(Task(taskFunc), false, signal);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_OK));
    ASSERT_EQ(m_threadManager->m_threadList.size(), 1);
}

TEST_F(ThreadManagerIfTest, startAllThreadsTestCase1)
{
    /***
     * Test scenario: test start successfully 2 threads and check keys if they are set to EXPECTED_KEY_VALUE.
     */
    auto signal1 = std::make_shared<Signal>();
    signal1->setTimeout(500);
    TaskFuncArgs args1 {0, &signal1->cond, &signal1->mtx, false, false, 0};
    
    auto signal2 = std::make_shared<Signal>();
    signal2->setTimeout(500);
    TaskFuncArgs args2 {0, &signal2->cond, &signal2->mtx, false, false, 0};
    
    m_threadManager->m_threadList.emplace_back(Task(taskFunc, &args1), true, signal1);
    m_threadManager->m_threadList.emplace_back(Task(taskFunc, &args2), false, signal2);
    
    auto rc = m_threadManager->startAllThreads();
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_OK));
    usleep(5000); /* Small sleep 1ms to make sure two thread have successfully set keys. */
    ASSERT_EQ(args1.key, EXPECTED_KEY_VALUE);
    ASSERT_EQ(args2.key, EXPECTED_KEY_VALUE);
    for(const auto &thr : m_threadManager->m_threadList)
    {
        m_cWrapperIfMock->cPthreadCancel(thr.tid);
        m_cWrapperIfMock->cPthreadDetach(thr.tid);
    }
}

TEST_F(ThreadManagerIfTest, startAllThreadsTestCase2)
{
    /***
     * Test scenario: test one of the threads gets destroyed before starting via ThreadManager,
     * then std::weak_ptr.lock() of a Signal is nullptr.
     */
    auto signal1 = std::make_shared<Signal>();
    signal1->setTimeout(500);
    TaskFuncArgs args1 {0, &signal1->cond, &signal1->mtx, false, false, 0};
    
    auto signal2 = std::make_shared<Signal>();
    signal2->setTimeout(500);
    TaskFuncArgs args2 {0, &signal2->cond, &signal2->mtx, false, false, 0};
    
    m_threadManager->m_threadList.emplace_back(Task(taskFunc, &args1), true, signal1);
    m_threadManager->m_threadList.emplace_back(Task(taskFunc, &args2), false, signal2);
    
    signal2.reset();
    auto rc = m_threadManager->startAllThreads();
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_SYSCALL_ERROR));
    ASSERT_EQ(args1.key, EXPECTED_KEY_VALUE);
    ASSERT_EQ(args2.key, 0);
    m_cWrapperIfMock->cPthreadCancel(m_threadManager->m_threadList.at(0).tid);
    m_cWrapperIfMock->cPthreadDetach(m_threadManager->m_threadList.at(0).tid);
}

TEST_F(ThreadManagerIfTest, startAllThreadsTestCase3)
{
    /***
     * Test scenario: test force causing thread initialisation timeout,
     * then pthread_cond_timedwait is expired.
     */
    bool forceTimeout = true;
    auto signal = std::make_shared<Signal>();
    signal->setTimeout(10); /* Make pthread_cond_timedwait timeout too small, so expires quickly */
    TaskFuncArgs args {0, &signal->cond, &signal->mtx, forceTimeout, false, 0};
    m_threadManager->m_threadList.emplace_back(Task(taskFunc, &args), true, signal);
    
    auto rc = m_threadManager->startAllThreads();
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_INITIALISATION_TIMEOUT));
    m_cWrapperIfMock->cPthreadCancel(m_threadManager->m_threadList.at(0).tid);
    m_cWrapperIfMock->cPthreadDetach(m_threadManager->m_threadList.at(0).tid);
}

TEST_F(ThreadManagerIfTest, terminateAllThreadsTestCase1)
{
    /***
     * Test scenario: test force pause via sleep() to create cancelation point,
     * so that terminateAllThreads can successfully send pthread_cancel requests.
     */
    bool forcePause = true;
    uint32_t pauseTime = 10; /* seconds */
    auto signal = std::make_shared<Signal>();
    signal->setTimeout(500);
    TaskFuncArgs args {0, &signal->cond, &signal->mtx, false, forcePause, pauseTime};
    m_threadManager->m_threadList.emplace_back(Task(taskFunc, &args), true, signal);
    
    auto rc = m_threadManager->startAllThreads();
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_OK));
    
    rc = m_threadManager->terminateAllThreads();
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_OK));
    ASSERT_EQ(args.key, 0);
}


} // namespace INTERNAL
} // namespace ItcPlatform