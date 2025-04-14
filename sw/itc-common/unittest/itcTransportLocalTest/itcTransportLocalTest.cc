#include "itcTransportLocal.h"

#include <iostream>
#include <memory>
#include <string>
#include <gtest/gtest.h>

#include "itcThreadPool.h"

namespace ITC
{
namespace INTERNAL
{
using namespace testing;
using namespace ITC::PROVIDED;
using ItcPlatformIfReturnCode = ItcPlatformIf::ItcPlatformIfReturnCode;

class ItcTransportLocalTest : public testing::Test
{
protected:
    ItcTransportLocalTest()
    {}

    ~ItcTransportLocalTest()
    {}
    
    void SetUp() override
    {
        m_regionId = 0x00200000;
        m_transportLocal = ItcTransportLocal::getInstance().lock();
        SmartContainer<ItcMailbox>::Callback callback = {
            [](const ItcMailbox &mbox) -> std::string
            {
                return mbox.name;
            },
            [](ItcMailbox &mbox)
            {
                mbox.reset();
            }
        };
        m_mboxList = std::make_shared<SmartContainer<ItcMailbox>>(callback);
        m_transportLocal->m_mboxList = m_mboxList;
        if(auto indexOpt = m_transportLocal->m_mboxList.lock()->emplace(ItcMailbox("sender")); indexOpt.has_value())
        {
            if(auto mboxOpt = m_transportLocal->m_mboxList.lock()->at(indexOpt.value()); mboxOpt.has_value())
            {
                m_sender = m_regionId | (indexOpt.value() & ITC_MASK_UNIT_ID);
                mboxOpt.value().get().mailboxId = m_sender;
                m_senderMbox = m_transportLocal->m_mboxList.lock()->data(indexOpt.value());
            }
        }
        if(auto indexOpt = m_transportLocal->m_mboxList.lock()->emplace(ItcMailbox("receiver")); indexOpt.has_value())
        {
            if(auto mboxOpt = m_transportLocal->m_mboxList.lock()->at(indexOpt.value()); mboxOpt.has_value())
            {
                m_receiver = m_regionId | (indexOpt.value() & ITC_MASK_UNIT_ID);
                mboxOpt.value().get().mailboxId = m_receiver;
                m_receiverMbox = m_transportLocal->m_mboxList.lock()->data(indexOpt.value());
            }
        }
        m_threadPool = std::make_shared<ThreadPool>(5);
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
    }

protected:
    std::shared_ptr<ItcTransportLocal> m_transportLocal  {nullptr};
    itc_mailbox_id_t m_regionId {ITC_MAILBOX_ID_DEFAULT};
    itc_mailbox_id_t m_sender {ITC_MAILBOX_ID_DEFAULT};
    itc_mailbox_id_t m_receiver {ITC_MAILBOX_ID_DEFAULT};
    std::shared_ptr<SmartContainer<ItcMailbox>> m_mboxList {nullptr};
    std::shared_ptr<ThreadPool> m_threadPool {nullptr};
    ItcMailboxRawPtr m_senderMbox {nullptr};
    ItcMailboxRawPtr m_receiverMbox {nullptr};
};

TEST_F(ItcTransportLocalTest, sendTest1)
{
    /***
     * Test scenario: test send a message to a mailbox.
     */
    uint32_t msgno = 0xAAAABBBB;
    auto sentMessage = ItcAdminMessageHelper::allocate(msgno);
    sentMessage->receiver = m_receiver;
    sentMessage->sender = m_sender;
    
    auto rc = m_transportLocal->send(sentMessage);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_OK));
    auto mboxOpt = m_transportLocal->m_mboxList.lock()->at("receiver");
    ASSERT_EQ(mboxOpt.has_value(), true);
    auto &mbox = mboxOpt.value().get();
    ASSERT_EQ(mbox.m_rxMsgQueue.size(), 1);
    ASSERT_EQ(mbox.m_rxMsgQueue.front(), sentMessage);
}

TEST_F(ItcTransportLocalTest, sendReceiveTest1)
{
    /***
     * Test scenario: test send/receive a message to/from a mailbox.
     */
    uint32_t msgno = 0xAAAABBBB;
    auto sentMessage = ItcAdminMessageHelper::allocate(msgno);
    sentMessage->receiver = m_receiver;
    sentMessage->sender = m_sender;
    
    auto rc = m_transportLocal->send(sentMessage);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_OK));
    auto mboxOpt = m_transportLocal->m_mboxList.lock()->at("receiver");
    ASSERT_EQ(mboxOpt.has_value(), true);
    auto &mbox = mboxOpt.value().get();
    ASSERT_EQ(mbox.m_rxMsgQueue.size(), 1);
    ASSERT_EQ(mbox.m_rxMsgQueue.front(), sentMessage);
    
    auto receivedMsg = m_transportLocal->receive(m_receiverMbox, ITC_MODE_TIMEOUT_NO_WAIT);
    ASSERT_EQ(receivedMsg, sentMessage);
    ItcAdminMessageHelper::deallocate(receivedMsg);
}

TEST_F(ItcTransportLocalTest, sendReceiveTest2)
{
    /***
     * Test scenario: test one thread sends and another thread receives a message.
     */
    
    std::vector<std::future<ItcAdminMessageRawPtr>> results;
    results.emplace_back(
        m_threadPool->enqueue(
            /* receiver thread */
            [this]()
            {
                return this->m_transportLocal->receive(m_receiverMbox);
            }
        )
    );
    
    uint32_t msgno = 0xAAAABBBB;
    auto sentMessage = ItcAdminMessageHelper::allocate(msgno);
    sentMessage->receiver = m_receiver;
    sentMessage->sender = m_sender;
    
    results.emplace_back(
        m_threadPool->enqueue(
            /* sender thread */
            [this, sentMessage]()
            {
                m_transportLocal->send(sentMessage);
                return static_cast<ItcAdminMessageRawPtr>(nullptr);
            }
        )
    );
    
    ASSERT_EQ(results.front().get(), sentMessage);
    ItcAdminMessageHelper::deallocate(sentMessage);
}

TEST_F(ItcTransportLocalTest, sendReceiveTest3)
{
    /***
     * Test scenario: test multiple threads send and one thread receives a messages.
     */
    std::vector<std::future<bool>> results;
    std::mutex receivedMsgsMutex;
    uint32_t NUMBER_OF_MESSAGES = 20;
    std::vector<ItcAdminMessageRawPtr> receivedMsgs;
    receivedMsgs.reserve(10);
    results.emplace_back(
        m_threadPool->enqueue(
            /* receiver thread */
            [this, &receivedMsgs, &receivedMsgsMutex, NUMBER_OF_MESSAGES]()
            {
                while(true)
                {
                    auto receivedMsg = this->m_transportLocal->receive(m_receiverMbox);
                    
                    std::scoped_lock lock(receivedMsgsMutex);
                    receivedMsgs.emplace_back(receivedMsg);
                    if(receivedMsgs.size() >= NUMBER_OF_MESSAGES)
                    {
                        break;
                    }
                }
                return true;
            }
        )
    );
    
    uint32_t msgno = 0xAAAABBBB;
    for(size_t i = 0; i < NUMBER_OF_MESSAGES; ++i)
    {
        m_threadPool->enqueue(
            /* sender thread */
            [this, msgno, i]()
            {
                auto sentMessage = ItcAdminMessageHelper::allocate(msgno);
                sentMessage->receiver = this->m_receiver;
                sentMessage->sender = this->m_sender;
                this->m_transportLocal->send(sentMessage);
            }
        );
    }
    
    results.front().get();
    
    ASSERT_EQ(receivedMsgs.size(), NUMBER_OF_MESSAGES);
    for(size_t i = 0; i < NUMBER_OF_MESSAGES; ++i)
    {
        std::scoped_lock lock(receivedMsgsMutex);
        ItcAdminMessageHelper::deallocate(receivedMsgs.at(i));
    }
}

TEST_F(ItcTransportLocalTest, sendReceiveTest4)
{
    /***
     * Test scenario: test sender and receiver mailboxes communicate to each other.
     */
    enum MESSAGE_SIGNAL_NUMBER
    {
        MODULE_XYZ_INTERFACE_ABC_SETUP_REQ = 0xAAAABBBB,
        MODULE_XYZ_INTERFACE_ABC_SETUP_CFM,
        MODULE_XYZ_INTERFACE_ABC_ACTIVATE_REQ,
        MODULE_XYZ_INTERFACE_ABC_ACTIVATE_CFM
    };
    
    m_threadPool->enqueue(
        /* receiver thread */
        [this]()
        {
            while(true)
            {
                auto receivedMsg = this->m_transportLocal->receive(m_receiverMbox);
                if(receivedMsg)
                {
                    switch (receivedMsg->msgno)
                    {
                    case MODULE_XYZ_INTERFACE_ABC_SETUP_REQ:
                        {
                            auto sentMessage = ItcAdminMessageHelper::allocate(MODULE_XYZ_INTERFACE_ABC_SETUP_CFM);
                            sentMessage->receiver = this->m_sender;
                            sentMessage->sender = this->m_receiver;
                            this->m_transportLocal->send(sentMessage);
                            ItcAdminMessageHelper::deallocate(receivedMsg);
                            break;
                        }
                    
                    case MODULE_XYZ_INTERFACE_ABC_ACTIVATE_REQ:
                        {
                            auto sentMessage = ItcAdminMessageHelper::allocate(MODULE_XYZ_INTERFACE_ABC_ACTIVATE_CFM);
                            sentMessage->receiver = this->m_sender;
                            sentMessage->sender = this->m_receiver;
                            this->m_transportLocal->send(sentMessage);
                            ItcAdminMessageHelper::deallocate(receivedMsg);
                            return;
                        }
                    
                    default:
                        return;
                    }   
                }
            }
        }
    );
    
    auto isFinished = m_threadPool->enqueue(
        /* sender thread */
        [this]()
        {
            auto sentMessage = ItcAdminMessageHelper::allocate(MODULE_XYZ_INTERFACE_ABC_SETUP_REQ);
            sentMessage->receiver = this->m_receiver;
            sentMessage->sender = this->m_sender;
            this->m_transportLocal->send(sentMessage);
            
            while(true)
            {
                auto receivedMsg = this->m_transportLocal->receive(m_senderMbox);
                if(receivedMsg)
                {
                    switch (receivedMsg->msgno)
                    {
                    case MODULE_XYZ_INTERFACE_ABC_SETUP_CFM:
                        {
                            auto sentMessage = ItcAdminMessageHelper::allocate(MODULE_XYZ_INTERFACE_ABC_ACTIVATE_REQ);
                            sentMessage->receiver = this->m_receiver;
                            sentMessage->sender = this->m_sender;
                            this->m_transportLocal->send(sentMessage);
                            ItcAdminMessageHelper::deallocate(receivedMsg);
                            break;
                        }
                    
                    case MODULE_XYZ_INTERFACE_ABC_ACTIVATE_CFM:
                        {
                            ItcAdminMessageHelper::deallocate(receivedMsg);
                            return true;
                        }
                    
                    default:
                        return false;
                    }
                }
            }
            return false;
        }
    );
    
    ASSERT_EQ(isFinished.get(), true);
}

} // namespace INTERNAL
} // namespace ITC