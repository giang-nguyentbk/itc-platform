#pragma once

#include "itcThreadManagerIf.h"

#include <gmock/gmock.h>

namespace ItcPlatform
{
namespace INTERNAL
{
using ThreadManagerIfReturnCode = ThreadManagerIf::ThreadManagerIfReturnCode;

class ThreadManagerIfMock : public ThreadManagerIf
{
public:
    static std::weak_ptr<ThreadManagerIfMock> getInstance();
    virtual ~ThreadManagerIfMock() = default;
    
    MOCK_METHOD(ThreadManagerIfReturnCode, setThreadAttributes, (int32_t policy, int32_t selfLimitPrio, int32_t priority), (override));
    MOCK_METHOD(ThreadManagerIfReturnCode, addThread, (const Task &task, bool useHighestPriority, std::shared_ptr<Signal> &signal), (override));
    MOCK_METHOD(ThreadManagerIfReturnCode, startAllThreads, (), (override));
    MOCK_METHOD(ThreadManagerIfReturnCode, terminateAllThreads, (), (override));

private:
    ThreadManagerIfMock() = default;

private:
    static std::shared_ptr<ThreadManagerIfMock> m_instance;
	static std::mutex m_singletonMutex;
    
    friend class ItcTransportSysvMsgQueueTest;
}; // class ThreadManagerIfMock



} // namespace INTERNAL
} // namespace ItcPlatform