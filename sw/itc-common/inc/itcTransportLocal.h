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
#include "itcThreadManager.h"
#include "itcTransportIf.h"

namespace ItcPlatform
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

using namespace ItcPlatform::PROVIDED;

struct UnitInfo
{
    itc_mailbox_id_t mailboxId {ITC_NO_MAILBOX_ID};
    uint32_t flags {0};
    bool isInUse {false};
    std::queue<ItcAdminMessageRawPtr> rxMsgQueue;
    UnitInfo(itc_mailbox_id_t iMailboxId = ITC_NO_MAILBOX_ID, uint32_t iFlags = 0, bool iIsInUse = false)
        : mailboxId(iMailboxId),
          flags(iFlags),
          isInUse(iIsInUse)
    {}
    ~UnitInfo() = default;
};

/***
 * This transport is to exchange ItcAdminMessage between mailboxes inside a Region only.
 */
class ItcTransportLocal : public ItcTransportIf
{
public:
    static std::weak_ptr<ItcTransportLocal> getInstance();
    virtual ~ItcTransportLocal() = default;
    
    ItcTransportLocal(const ItcTransportLocal &other) = delete;
    ItcTransportLocal &operator=(const ItcTransportLocal &other) = delete;
    ItcTransportLocal(ItcTransportLocal &&other) noexcept = delete;
    ItcTransportLocal &operator=(ItcTransportLocal &&other) noexcept = delete;
    
    ItcTransportIfReturnCode initialise(itc_mailbox_id_t regionId = ITC_NO_MAILBOX_ID, uint32_t mboxCount = 0, uint32_t flags = ITC_NO_MAILBOX_ID) override;
    ItcTransportIfReturnCode release() override;
    ItcTransportIfReturnCode locateItcServer(itc_mailbox_id_t *assignedRegionId, itc_mailbox_id_t *locatedItcServerMboxId) override;
    ItcTransportIfReturnCode createMailbox(ItcMailboxRawPtr newMbox) override;
    ItcTransportIfReturnCode deleteMailbox(ItcMailboxRawPtr mbox) override;
    ItcTransportIfReturnCode sendMessage(ItcAdminMessageRawPtr adminMsg, const MailboxContactInfo &toMbox) override;
    ItcAdminMessageRawPtr receiveMessage(ItcMailboxRawPtr myMbox) override;
    size_t getMaxMessageSize() override;
    
private:
    ItcTransportLocal() = default;
    
    void releaseUnitInfo(ssize_t atIndex);
    ssize_t getUnitInfoIndex(uint32_t mboxId);
    
    friend std::shared_ptr<ItcTransportLocal>;
    
private:
    static std::shared_ptr<ItcTransportLocal> m_instance;
	static std::mutex m_singletonMutex;
    itc_mailbox_id_t            m_regionId {ITC_NO_MAILBOX_ID};
    uint32_t                    m_unitCount {0};
    std::vector<UnitInfo>       m_unitInfos;
    
    friend class ItcTransportLocalTest;
	FRIEND_TEST(ItcTransportLocalTest, getUnitInfoIndexTest1);
	FRIEND_TEST(ItcTransportLocalTest, getUnitInfoIndexTest2);
	FRIEND_TEST(ItcTransportLocalTest, getUnitInfoIndexTest3);
	FRIEND_TEST(ItcTransportLocalTest, getUnitInfoIndexTest4);
	FRIEND_TEST(ItcTransportLocalTest, releaseUnitInfoTest1);
	FRIEND_TEST(ItcTransportLocalTest, initialiseTest1);
	FRIEND_TEST(ItcTransportLocalTest, initialiseTest2);
	FRIEND_TEST(ItcTransportLocalTest, initialiseTest3);
	FRIEND_TEST(ItcTransportLocalTest, releaseTest1);
	FRIEND_TEST(ItcTransportLocalTest, releaseTest2);
	FRIEND_TEST(ItcTransportLocalTest, createMailboxTest1);
	FRIEND_TEST(ItcTransportLocalTest, createMailboxTest2);
	FRIEND_TEST(ItcTransportLocalTest, deleteMailboxTest1);
	FRIEND_TEST(ItcTransportLocalTest, deleteMailboxTest2);
	FRIEND_TEST(ItcTransportLocalTest, sendMessageTest1);
	FRIEND_TEST(ItcTransportLocalTest, sendMessageTest2);
	FRIEND_TEST(ItcTransportLocalTest, receiveMessageTest1);
	FRIEND_TEST(ItcTransportLocalTest, receiveMessageTest2);
}; // class ItcTransportLocal

} // namespace INTERNAL
} // namespace ItcPlatform