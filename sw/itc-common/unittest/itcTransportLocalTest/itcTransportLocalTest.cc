#include "itcTransportLocal.h"

#include <iostream>
#include <memory>
#include <string>
#include <chrono>
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
        auto callback = [&](ItcMailboxRawPtr mailbox, uint32_t index)
        {
            mailbox->m_mailboxId = (m_regionId << ITC_REGION_ID_SHIFT) | (index & ITC_MASK_UNIT_ID);
        };
        m_mboxList = std::make_shared<ConcurrentContainer<ItcMailbox, ITC_MAX_SUPPORTED_MAILBOXES>>(callback);
        m_sender = m_mboxList->tryPopFromQueue();
        m_mboxList->addEntryToHashMap("sender", m_sender);
        m_sender->setState(true);
        m_receiver = m_mboxList->tryPopFromQueue();
        m_mboxList->addEntryToHashMap("receiver", m_receiver);
        m_receiver->setState(true);
        m_transportLocal->initialise(m_mboxList);
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
    ItcMailboxRawPtr m_sender {nullptr};
    ItcMailboxRawPtr m_receiver {nullptr};
    std::shared_ptr<ConcurrentContainer<ItcMailbox, ITC_MAX_SUPPORTED_MAILBOXES>> m_mboxList {nullptr};
};

TEST_F(ItcTransportLocalTest, test1)
{
    /***
     * Test scenario: test send a message to a mailbox.
     */
    uint32_t msgno = 0xAAAABBBB;
    auto sentMessage = ItcAdminMessageHelper::allocate(msgno);
    sentMessage->receiver = m_receiver->m_mailboxId;
    sentMessage->sender = m_sender->m_mailboxId;
    std::chrono::_V2::system_clock::time_point start;
    std::chrono::_V2::system_clock::time_point end;
    
    start = std::chrono::high_resolution_clock::now();
    MAYBE_UNUSED auto rc = m_transportLocal->send(sentMessage);
    // ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_OK));
    auto receivedMessage = m_transportLocal->receive(m_receiver, ITC_MODE_RECEIVE_NON_BLOCKING);
    end = std::chrono::high_resolution_clock::now();
    ASSERT_EQ(receivedMessage, sentMessage);
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "[BENCHMARK] ItcTransportLocalTest test1 " << " took " << duration << " ns\n";
}

TEST_F(ItcTransportLocalTest, test2)
{
    /***
     * Test scenario: test one thread sends and another thread receives a message.
     */
    std::chrono::_V2::system_clock::time_point start;
    std::chrono::_V2::system_clock::time_point end;
    std::chrono::_V2::system_clock::time_point start1;
    std::chrono::_V2::system_clock::time_point end1;
    constexpr uint32_t NUMBER_OF_MESSAGES = 1000;
    std::array<ItcAdminMessageRawPtr, NUMBER_OF_MESSAGES> receivedMessages;
    auto receiver = [&]()
    {
        uint32_t count {0};
        while(count < NUMBER_OF_MESSAGES)
        {
            auto msg = m_transportLocal->receive(m_receiver);
            if(msg)
            {
                receivedMessages.at(count) = msg;
                ++count;
            }
        }
    };
    
    auto sender = [&]()
    {
        start1 = std::chrono::high_resolution_clock::now();
        std::array<ItcAdminMessageRawPtr, NUMBER_OF_MESSAGES> sentMessages;
        for(uint32_t i = 0; i < NUMBER_OF_MESSAGES; ++i)
        {
            sentMessages.at(i) = ItcAdminMessageHelper::allocate(0xFFFF + i);
            sentMessages.at(i)->receiver = m_receiver->m_mailboxId;
            sentMessages.at(i)->sender = m_sender->m_mailboxId;
        }
        end1 = std::chrono::high_resolution_clock::now();
        
        for(uint32_t i = 0; i < NUMBER_OF_MESSAGES; ++i)
        {
            m_transportLocal->send(sentMessages.at(i));
            sentMessages.at(i) = nullptr;
        }
    };

    start = std::chrono::high_resolution_clock::now();
    std::thread receiverThread(receiver);
    std::thread senderThread(sender);
    receiverThread.join();
    senderThread.join();
    end = std::chrono::high_resolution_clock::now();
    
    for(uint32_t i = 0; i < NUMBER_OF_MESSAGES; ++i)
    {
        ItcAdminMessageHelper::deallocate(receivedMessages[i]);
    }
    
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>((end - start) - (end1 - start1)).count();
    std::cout << "[BENCHMARK] ItcTransportLocalTest test2 " << " took " << duration / NUMBER_OF_MESSAGES << " ns\n";
}

// TEST_F(ItcTransportLocalTest, sendReceiveTest3)
// {
//     /***
//      * Test scenario: test multiple threads send and one thread receives a messages.
//      */
//     std::vector<std::future<bool>> results;
//     std::mutex receivedMsgsMutex;
//     uint32_t NUMBER_OF_MESSAGES = 20;
//     std::vector<ItcAdminMessageRawPtr> receivedMsgs;
//     receivedMsgs.reserve(10);
//     results.emplace_back(
//         m_threadPool->enqueue(
//             /* receiver thread */
//             [this, &receivedMsgs, &receivedMsgsMutex, NUMBER_OF_MESSAGES]()
//             {
//                 while(true)
//                 {
//                     auto receivedMsg = this->m_transportLocal->receive(m_receiverMbox);
                    
//                     std::scoped_lock lock(receivedMsgsMutex);
//                     receivedMsgs.emplace_back(receivedMsg);
//                     if(receivedMsgs.size() >= NUMBER_OF_MESSAGES)
//                     {
//                         break;
//                     }
//                 }
//                 return true;
//             }
//         )
//     );
    
//     uint32_t msgno = 0xAAAABBBB;
//     for(size_t i = 0; i < NUMBER_OF_MESSAGES; ++i)
//     {
//         m_threadPool->enqueue(
//             /* sender thread */
//             [this, msgno, i]()
//             {
//                 auto sentMessage = ItcAdminMessageHelper::allocate(msgno);
//                 sentMessage->receiver = this->m_receiver;
//                 sentMessage->sender = this->m_sender;
//                 this->m_transportLocal->send(sentMessage);
//             }
//         );
//     }
    
//     results.front().get();
    
//     ASSERT_EQ(receivedMsgs.size(), NUMBER_OF_MESSAGES);
//     for(size_t i = 0; i < NUMBER_OF_MESSAGES; ++i)
//     {
//         std::scoped_lock lock(receivedMsgsMutex);
//         ItcAdminMessageHelper::deallocate(receivedMsgs.at(i));
//     }
// }

// TEST_F(ItcTransportLocalTest, sendReceiveTest4)
// {
//     /***
//      * Test scenario: test sender and receiver mailboxes communicate to each other.
//      */
//     enum MESSAGE_SIGNAL_NUMBER
//     {
//         MODULE_XYZ_INTERFACE_ABC_SETUP_REQ = 0xAAAABBBB,
//         MODULE_XYZ_INTERFACE_ABC_SETUP_CFM,
//         MODULE_XYZ_INTERFACE_ABC_ACTIVATE_REQ,
//         MODULE_XYZ_INTERFACE_ABC_ACTIVATE_CFM
//     };
    
//     m_threadPool->enqueue(
//         /* receiver thread */
//         [this]()
//         {
//             while(true)
//             {
//                 auto receivedMsg = this->m_transportLocal->receive(m_receiverMbox);
//                 if(receivedMsg)
//                 {
//                     switch (receivedMsg->msgno)
//                     {
//                     case MODULE_XYZ_INTERFACE_ABC_SETUP_REQ:
//                         {
//                             auto sentMessage = ItcAdminMessageHelper::allocate(MODULE_XYZ_INTERFACE_ABC_SETUP_CFM);
//                             sentMessage->receiver = this->m_sender;
//                             sentMessage->sender = this->m_receiver;
//                             this->m_transportLocal->send(sentMessage);
//                             ItcAdminMessageHelper::deallocate(receivedMsg);
//                             break;
//                         }
                    
//                     case MODULE_XYZ_INTERFACE_ABC_ACTIVATE_REQ:
//                         {
//                             auto sentMessage = ItcAdminMessageHelper::allocate(MODULE_XYZ_INTERFACE_ABC_ACTIVATE_CFM);
//                             sentMessage->receiver = this->m_sender;
//                             sentMessage->sender = this->m_receiver;
//                             this->m_transportLocal->send(sentMessage);
//                             ItcAdminMessageHelper::deallocate(receivedMsg);
//                             return;
//                         }
                    
//                     default:
//                         return;
//                     }   
//                 }
//             }
//         }
//     );
    
//     auto isFinished = m_threadPool->enqueue(
//         /* sender thread */
//         [this]()
//         {
//             auto sentMessage = ItcAdminMessageHelper::allocate(MODULE_XYZ_INTERFACE_ABC_SETUP_REQ);
//             sentMessage->receiver = this->m_receiver;
//             sentMessage->sender = this->m_sender;
//             this->m_transportLocal->send(sentMessage);
            
//             while(true)
//             {
//                 auto receivedMsg = this->m_transportLocal->receive(m_senderMbox);
//                 if(receivedMsg)
//                 {
//                     switch (receivedMsg->msgno)
//                     {
//                     case MODULE_XYZ_INTERFACE_ABC_SETUP_CFM:
//                         {
//                             auto sentMessage = ItcAdminMessageHelper::allocate(MODULE_XYZ_INTERFACE_ABC_ACTIVATE_REQ);
//                             sentMessage->receiver = this->m_receiver;
//                             sentMessage->sender = this->m_sender;
//                             this->m_transportLocal->send(sentMessage);
//                             ItcAdminMessageHelper::deallocate(receivedMsg);
//                             break;
//                         }
                    
//                     case MODULE_XYZ_INTERFACE_ABC_ACTIVATE_CFM:
//                         {
//                             ItcAdminMessageHelper::deallocate(receivedMsg);
//                             return true;
//                         }
                    
//                     default:
//                         return false;
//                     }
//                 }
//             }
//             return false;
//         }
//     );
    
//     ASSERT_EQ(isFinished.get(), true);
// }

} // namespace INTERNAL
} // namespace ITC