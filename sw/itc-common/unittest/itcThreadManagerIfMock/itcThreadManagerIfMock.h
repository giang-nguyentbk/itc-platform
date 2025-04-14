#pragma once

#include "itcThreadManagerIf.h"
#include "itcConstant.h"

#include <gmock/gmock.h>

namespace ITC
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
    MOCK_METHOD(ThreadManagerIfReturnCode, addThread, (const Task &task, std::shared_ptr<SyncObject> syncObj, bool useHighestPriority), (override));
    MOCK_METHOD(ThreadManagerIfReturnCode, startAllThreads, (), (override));
    MOCK_METHOD(ThreadManagerIfReturnCode, terminateAllThreads, (), (override));

private:
    ThreadManagerIfMock() = default;

private:
    SINGLETON_DECLARATION(ThreadManagerIfMock)
    
    friend class ItcTransportSysvMsgQueueTest;
}; // class ThreadManagerIfMock



} // namespace INTERNAL
} // namespace ITC