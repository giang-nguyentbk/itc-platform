#pragma once

#include <cstdint>
#include <cstddef>
#include <queue>
#include <vector>
#include <memory>
#include <mutex>

#include <gtest/gtest.h>
// #include <enumUtils.h>

#include "itc.h"
#include "itcConstant.h"
#include "itcMailbox.h"
#include "itcAdminMessage.h"
#include "itcConcurrentContainer.h"

namespace ITC
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

using namespace ITC::PROVIDED;
using ItcPlatformIfReturnCode = ItcPlatformIf::ItcPlatformIfReturnCode;

/***
 * This transport is to exchange ItcAdminMessage between mailboxes inside a Region only.
 */
class ItcTransportLocal
{
public:
    static std::weak_ptr<ItcTransportLocal> getInstance();
    virtual ~ItcTransportLocal() = default;
    
    ItcTransportLocal(const ItcTransportLocal &other) = delete;
    ItcTransportLocal &operator=(const ItcTransportLocal &other) = delete;
    ItcTransportLocal(ItcTransportLocal &&other) noexcept = delete;
    ItcTransportLocal &operator=(ItcTransportLocal &&other) noexcept = delete;
    
    bool initialise(std::shared_ptr<ConcurrentContainer<ItcMailbox, ITC_MAX_SUPPORTED_MAILBOXES>> mboxList, ItcMailboxRawPtr myMbox);
    
    ItcPlatformIfReturnCode send(ItcAdminMessageRawPtr adminMsg);
    ItcAdminMessageRawPtr receive(uint32_t mode = ITC_MODE_DEFAULT);
    
private:
    SINGLETON_DECLARATION(ItcTransportLocal)
    ItcTransportLocal() = default;
    
private:
    std::weak_ptr<ConcurrentContainer<ItcMailbox, ITC_MAX_SUPPORTED_MAILBOXES>> m_mboxList;
    ItcMailboxRawPtr m_myMbox {nullptr};
    
    friend class ItcTransportLocalTest;
	FRIEND_TEST(ItcTransportLocalTest, sendTest1);
	FRIEND_TEST(ItcTransportLocalTest, sendReceiveTest1);
	FRIEND_TEST(ItcTransportLocalTest, sendReceiveTest2);
	FRIEND_TEST(ItcTransportLocalTest, sendReceiveTest3);
	FRIEND_TEST(ItcTransportLocalTest, sendReceiveTest4);
}; // class ItcTransportLocal

} // namespace INTERNAL
} // namespace ITC