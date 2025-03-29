#include "itcTransportLocal.h"

#include <iostream>
#include <memory>
#include <string>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "itcPlatformIfMock.h"

namespace ItcPlatform
{
namespace INTERNAL
{
using namespace testing;
using ::testing::AtLeast;
using namespace ItcPlatform::PROVIDED;
using ItcPlatformIfReturnCode = ItcPlatformIf::ItcPlatformIfReturnCode;
using ItcTransportIfReturnCode = ItcTransportIf::ItcTransportIfReturnCode;

class ItcTransportLocalTest : public testing::Test
{
protected:
    ItcTransportLocalTest()
    {}

    ~ItcTransportLocalTest()
    {}
    
    void SetUp() override
    {
        m_transportLocal = ItcTransportLocal::getInstance().lock();
        m_itcPlatformIfMock = ItcPlatformIfMock::getInstance().lock();
    }

    void TearDown() override
    {
        /***
         * We can release the ownership of m_transportLocal from the ItcTransportLocal static instance,
         * user_count decreases by 1.
         * 
         * Actually, in unit test, we can access to private static data member m_instance and call reset() on it
         * to destroy Singleton object that loses the meaning of Singleton, but that's only in unit test because of
         * FRIEND_TEST and friend class, in production, no way to access private field m_instance,
         * so Singleton shared_ptr is still preserved till process termination.
         * 
         * In unit test, we should make Singleton refresh after every single TEST_F. So, let's access to m_instance,
         * and reset() it on purpose.
         */
        m_transportLocal->m_instance.reset();
        m_itcPlatformIfMock->m_instance.reset();
    }

protected:
    std::shared_ptr<ItcTransportLocal> m_transportLocal;
    std::shared_ptr<ItcPlatformIfMock> m_itcPlatformIfMock;
};

TEST_F(ItcTransportLocalTest, getUnitInfoIndexTest1)
{
    /***
     * Test scenario: test did not call initialise() yet.
     */
    ssize_t index = m_transportLocal->getUnitInfoIndex(ITC_NO_MAILBOX_ID);
    ASSERT_EQ(index, -1);
}

TEST_F(ItcTransportLocalTest, getUnitInfoIndexTest2)
{
    /***
     * Test scenario: test wrong regionId.
     */
    itc_mailbox_id_t mboxId = 0x00100001;
    m_transportLocal->m_regionId = 0x00200000;
    m_transportLocal->m_unitInfos.emplace_back(mboxId, ITC_FLAG_NO_FLAG, true);
    
    ssize_t index = m_transportLocal->getUnitInfoIndex(mboxId);
    ASSERT_EQ(index, -2);
}

TEST_F(ItcTransportLocalTest, getUnitInfoIndexTest3)
{
    /***
     * Test scenario: test unitId exceeds m_unitCount.
     */
    itc_mailbox_id_t mboxId = 0x0010000A;
    m_transportLocal->m_regionId = 0x00100000;
    m_transportLocal->m_unitCount = 9;
    m_transportLocal->m_unitInfos.emplace_back(mboxId, ITC_FLAG_NO_FLAG, true);
    
    ssize_t index = m_transportLocal->getUnitInfoIndex(mboxId);
    ASSERT_EQ(index, -3);
}

TEST_F(ItcTransportLocalTest, getUnitInfoIndexTest4)
{
    /***
     * Test scenario: test happy case that successfully obtains UnitInfo's index in m_unitInfos.
     */
    itc_mailbox_id_t mboxId = 0x0010000A;
    m_transportLocal->m_regionId = 0x00100000;
    m_transportLocal->m_unitCount = 100;
    m_transportLocal->m_unitInfos.emplace_back(mboxId, ITC_FLAG_NO_FLAG, true);
    
    ssize_t index = m_transportLocal->getUnitInfoIndex(mboxId);
    ASSERT_EQ(index, 0xA);
}

TEST_F(ItcTransportLocalTest, releaseUnitInfoTest1)
{
    /***
     * Test scenario: test happy case that drops out one ItcAdminMessageRawPtr in the rx queue.
     */
    itc_mailbox_id_t mboxId = 0x00100000;
    m_transportLocal->m_regionId = 0x00100000;
    m_transportLocal->m_unitCount = 100;
    m_transportLocal->m_unitInfos.emplace_back(mboxId, ITC_FLAG_NO_FLAG, true);
    auto &unitInfo = m_transportLocal->m_unitInfos.front();
    itc_mailbox_id_t sender = 0x00300001;
    itc_mailbox_id_t receiver = 0x00300004;
    uint32_t msgno = 0x0000FFFF;
    auto adminMsg = new ItcAdminMessage(sender, receiver, ITC_FLAG_NO_FLAG, sizeof(msgno), msgno);
    unitInfo.rxMsgQueue.push(adminMsg);
    EXPECT_CALL(*m_itcPlatformIfMock, deleteMessage(
        Truly([&adminMsg](ItcMessageRawPtr msg)
        {
            ItcAdminMessageRawPtr adminMsgInArg = CONVERT_TO_ADMIN_MESSAGE(msg);
            bool isMatchedExpectation = adminMsgInArg->sender == adminMsg->sender &&
                                        adminMsgInArg->receiver == adminMsg->receiver &&
                                        adminMsgInArg->msgno == adminMsg->msgno;
            delete adminMsgInArg;
            return isMatchedExpectation;
        })
    )).Times(1).WillOnce(Return(MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_OK)));
    
    ssize_t atIndex = 0;
    m_transportLocal->releaseUnitInfo(atIndex);
    ASSERT_EQ(unitInfo.mailboxId, ITC_NO_MAILBOX_ID);
    ASSERT_EQ(unitInfo.flags, 0);
    ASSERT_EQ(unitInfo.isInUse, false);
    ASSERT_EQ(unitInfo.rxMsgQueue.empty(), true);
}

TEST_F(ItcTransportLocalTest, initialiseTest1)
{
    /***
     * Test scenario: test accidentally call initialise(),
     * although already initialised without ITC_FLAG_FORCE_REINIT_TRANSPORTS.
     */
    itc_mailbox_id_t mboxId = 0x00100000;
    itc_mailbox_id_t regionId = 0x00100000;
    m_transportLocal->m_unitInfos.emplace_back(mboxId, ITC_FLAG_NO_FLAG, true);
    
    auto rc = m_transportLocal->initialise(regionId, 5, ITC_FLAG_NO_FLAG);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_ALREADY_INITIALISED));
}

TEST_F(ItcTransportLocalTest, initialiseTest2)
{
    /***
     * Test scenario: test ITC_FLAG_FORCE_REINIT_TRANSPORTS.
     */
    itc_mailbox_id_t mboxId = 0x00100000;
    itc_mailbox_id_t regionId = 0x00100000;
    uint32_t mboxCount = 5;
    uint32_t unitCount = (0xFFFFFFFF >> COUNT_LEADING_ZEROS(mboxCount)) + 1;
    m_transportLocal->m_unitInfos.emplace_back(mboxId, ITC_FLAG_NO_FLAG, true);
    m_transportLocal->m_unitCount = 1;
    auto &unitInfo = m_transportLocal->m_unitInfos.front();
    itc_mailbox_id_t sender = 0x00300001;
    itc_mailbox_id_t receiver = 0x00300004;
    uint32_t msgno = 0x0000FFFF;
    auto adminMsg = new ItcAdminMessage(sender, receiver, ITC_FLAG_NO_FLAG, sizeof(msgno), msgno);
    unitInfo.rxMsgQueue.push(adminMsg);
    ASSERT_EQ(m_transportLocal->m_unitInfos.size(), 1);
    ASSERT_EQ(unitInfo.rxMsgQueue.size(), 1);
    
    /* This EXPECT_CALL is for deleteMessage inside releaseUnitInfo */
    EXPECT_CALL(*m_itcPlatformIfMock, deleteMessage(
        Truly([&adminMsg](ItcMessageRawPtr msg)
        {
            ItcAdminMessageRawPtr adminMsgInArg = CONVERT_TO_ADMIN_MESSAGE(msg);
            bool isMatchedExpectation = adminMsgInArg->sender == adminMsg->sender &&
                                        adminMsgInArg->receiver == adminMsg->receiver &&
                                        adminMsgInArg->msgno == adminMsg->msgno;
            delete adminMsgInArg;
            return isMatchedExpectation;
        })
    )).Times(1).WillOnce(Return(MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_OK)));
    
    auto rc = m_transportLocal->initialise(regionId, mboxCount, ITC_FLAG_FORCE_REINIT_TRANSPORTS);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_OK));
    ASSERT_EQ(m_transportLocal->m_regionId, regionId);
    ASSERT_EQ(m_transportLocal->m_unitCount, unitCount);
    ASSERT_EQ(m_transportLocal->m_unitInfos.size(), unitCount);
}

TEST_F(ItcTransportLocalTest, initialiseTest3)
{
    /***
     * Test scenario: test a fresh initialise().
     */
    itc_mailbox_id_t regionId = 0x00100000;
    uint32_t mboxCount = 5;
    uint32_t unitCount = (0xFFFFFFFF >> COUNT_LEADING_ZEROS(mboxCount)) + 1;
    
    auto rc = m_transportLocal->initialise(regionId, mboxCount, ITC_FLAG_NO_FLAG);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_OK));
    ASSERT_EQ(m_transportLocal->m_regionId, regionId);
    ASSERT_EQ(m_transportLocal->m_unitCount, unitCount);
    ASSERT_EQ(m_transportLocal->m_unitInfos.size(), unitCount);
}

TEST_F(ItcTransportLocalTest, releaseTest1)
{
    /***
     * Test scenario: test a fresh release() with a simulated internal mailbox of ITC system.
     */
    m_transportLocal->m_unitInfos.emplace_back(ITC_NO_MAILBOX_ID, ITC_FLAG_NO_FLAG, true);
    
    auto rc = m_transportLocal->release();
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_OK));
    ASSERT_EQ(m_transportLocal->m_unitInfos.empty(), true);
}

TEST_F(ItcTransportLocalTest, releaseTest2)
{
    /***
     * Test scenario: test call release() when there are user-created running mailboxes.
     */
    m_transportLocal->m_unitInfos.emplace_back(ITC_NO_MAILBOX_ID, ITC_FLAG_NO_FLAG, true);
    m_transportLocal->m_unitInfos.emplace_back(ITC_NO_MAILBOX_ID, ITC_FLAG_NO_FLAG, true);
    
    auto rc = m_transportLocal->release();
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_INVALID_OPERATION));
    ASSERT_EQ(m_transportLocal->m_unitInfos.size(), 2);
}

TEST_F(ItcTransportLocalTest, createMailboxTest1)
{
    /***
     * Test scenario: test createMailbox() with a mailbox ID already occupied by another.
     */
    bool isAlreadyInUse = true;
    itc_mailbox_id_t mboxId = 0x000100000;
    m_transportLocal->m_regionId = 0x000100000;
    m_transportLocal->m_unitCount = 1;
    m_transportLocal->m_unitInfos.emplace_back(mboxId, ITC_FLAG_NO_FLAG, isAlreadyInUse);
    ItcMailbox mbox {"dummyMbox", mboxId, MailboxState::MBOX_STATE_INUSE};
    
    auto rc = m_transportLocal->createMailbox(&mbox);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_ALREADY_IN_USE));
}

TEST_F(ItcTransportLocalTest, createMailboxTest2)
{
    /***
     * Test scenario: test createMailbox() successfully.
     */
    itc_mailbox_id_t mboxId = 0x000100000;
    m_transportLocal->m_regionId = 0x000100000;
    m_transportLocal->m_unitCount = 1;
    m_transportLocal->m_unitInfos.emplace_back(mboxId, ITC_FLAG_NO_FLAG, false);
    auto &unitInfo = m_transportLocal->m_unitInfos.front();
    ItcMailbox mbox {"dummyMbox", mboxId, MailboxState::MBOX_STATE_INUSE};
    
    auto rc = m_transportLocal->createMailbox(&mbox);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_OK));
    ASSERT_EQ(unitInfo.mailboxId, mboxId);
    ASSERT_EQ(unitInfo.flags, ITC_FLAG_NO_FLAG);
    ASSERT_EQ(unitInfo.isInUse, true);
}

TEST_F(ItcTransportLocalTest, deleteMailboxTest1)
{
    /***
     * Test scenario: test deleteMailbox() with a mailbox ID not created yet.
     */
    bool isCreatedYet = false;
    itc_mailbox_id_t mboxId = 0x000100000;
    m_transportLocal->m_regionId = 0x000100000;
    m_transportLocal->m_unitCount = 1;
    m_transportLocal->m_unitInfos.emplace_back(mboxId, ITC_FLAG_NO_FLAG, isCreatedYet);
    ItcMailbox mbox {"dummyMbox", mboxId, MailboxState::MBOX_STATE_INUSE};
    
    auto rc = m_transportLocal->deleteMailbox(&mbox);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_INVALID_OPERATION));
}

TEST_F(ItcTransportLocalTest, deleteMailboxTest2)
{
    /***
     * Test scenario: test deleteMailbox() successfully.
     */
    itc_mailbox_id_t mboxId = 0x000100000;
    m_transportLocal->m_regionId = 0x000100000;
    m_transportLocal->m_unitCount = 1;
    m_transportLocal->m_unitInfos.emplace_back(mboxId, ITC_FLAG_NO_FLAG, true);
    ItcMailbox mbox {"dummyMbox", mboxId, MailboxState::MBOX_STATE_INUSE};
    
    auto rc = m_transportLocal->deleteMailbox(&mbox);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_OK));
}

TEST_F(ItcTransportLocalTest, sendMessageTest1)
{
    /***
     * Test scenario: test send messages to a not-created-yet mailbox.
     */
    bool isCreatedYet = false;
    itc_mailbox_id_t mboxId = 0x000100000;
    m_transportLocal->m_regionId = 0x000100000;
    m_transportLocal->m_unitCount = 1;
    m_transportLocal->m_unitInfos.emplace_back(mboxId, ITC_FLAG_NO_FLAG, isCreatedYet);
    ItcAdminMessageRawPtr adminMsg = new ItcAdminMessage;
    MailboxContactInfo toMboxInfo {ITC_NO_MAILBOX_ID, mboxId};
    
    auto rc = m_transportLocal->sendMessage(adminMsg, toMboxInfo);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_INVALID_OPERATION));
    delete adminMsg;
}

TEST_F(ItcTransportLocalTest, sendMessageTest2)
{
    /***
     * Test scenario: test send messages to a not-created-yet mailbox.
     */
    itc_mailbox_id_t mboxId = 0x000100000;
    m_transportLocal->m_regionId = 0x000100000;
    m_transportLocal->m_unitCount = 1;
    m_transportLocal->m_unitInfos.emplace_back(mboxId, ITC_FLAG_NO_FLAG, true);
    ItcAdminMessageRawPtr adminMsg = new ItcAdminMessage(ITC_NO_MAILBOX_ID, mboxId);
    MailboxContactInfo toMboxInfo {ITC_NO_MAILBOX_ID, mboxId};
    auto &unitInfo = m_transportLocal->m_unitInfos.front();
    
    auto rc = m_transportLocal->sendMessage(adminMsg, toMboxInfo);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_OK));
    ASSERT_EQ(unitInfo.rxMsgQueue.size(), 1);
    ASSERT_EQ(unitInfo.rxMsgQueue.front()->receiver, mboxId);
    delete adminMsg;
}

TEST_F(ItcTransportLocalTest, receiveMessageTest1)
{
    /***
     * Test scenario: test receive messages on a not-created-yet mailbox.
     */
    bool isCreatedYet = false;
    itc_mailbox_id_t mboxId = 0x000100000;
    m_transportLocal->m_regionId = 0x000100000;
    m_transportLocal->m_unitCount = 1;
    m_transportLocal->m_unitInfos.emplace_back(mboxId, ITC_FLAG_NO_FLAG, isCreatedYet);
    ItcMailbox mbox {"myMailbox", mboxId, MailboxState::MBOX_STATE_INUSE};
    
    auto receivedMsg = m_transportLocal->receiveMessage(&mbox);
    ASSERT_EQ(receivedMsg, nullptr);
}

TEST_F(ItcTransportLocalTest, receiveMessageTest2)
{
    /***
     * Test scenario: test send messages to a not-created-yet mailbox.
     */
    itc_mailbox_id_t mboxId = 0x000100000;
    m_transportLocal->m_regionId = 0x000100000;
    m_transportLocal->m_unitCount = 1;
    m_transportLocal->m_unitInfos.emplace_back(mboxId, ITC_FLAG_NO_FLAG, true);
    ItcAdminMessageRawPtr adminMsg = new ItcAdminMessage(ITC_NO_MAILBOX_ID, mboxId);
    ItcMailbox mbox {"myMailbox", mboxId, MailboxState::MBOX_STATE_INUSE};
    auto &unitInfo = m_transportLocal->m_unitInfos.front();
    unitInfo.rxMsgQueue.push(adminMsg);
    
    auto receivedMsg = m_transportLocal->receiveMessage(&mbox);
    ASSERT_EQ(receivedMsg, adminMsg);
    ASSERT_EQ(unitInfo.rxMsgQueue.size(), 0);
    delete adminMsg;
}

} // namespace INTERNAL
} // namespace ItcPlatform