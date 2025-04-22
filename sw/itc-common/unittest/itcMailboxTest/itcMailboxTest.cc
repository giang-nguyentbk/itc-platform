#include "itcMailbox.h"

#include <iostream>
#include <memory>
#include <string>
#include <chrono>
#include <thread>
#include <gtest/gtest.h>


namespace ITC
{
namespace INTERNAL
{
using namespace testing;
using namespace ITC::PROVIDED;

class ItcMailboxTest : public testing::Test
{
protected:
    ItcMailboxTest()
    {}

    ~ItcMailboxTest()
    {}
    
    void SetUp() override
    {}

    void TearDown() override
    {}

protected:
};

TEST_F(ItcMailboxTest, test1)
{
    /***
     * Test scenario: test send receive benchmarking.
     */
    ItcMailbox mbox;
    mbox.setState(true);
    auto sentMessage = ItcAdminMessageHelper::allocate(0xAAAABBBB);
    std::chrono::_V2::system_clock::time_point start;
    std::chrono::_V2::system_clock::time_point end;
    
    start = std::chrono::high_resolution_clock::now();
    MAYBE_UNUSED auto success = mbox.push(sentMessage);
    // ASSERT_EQ(success, true);
    auto msg = mbox.pop();
    end = std::chrono::high_resolution_clock::now();
    ASSERT_EQ(msg, sentMessage);
    ItcAdminMessageHelper::deallocate(msg);
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "[BENCHMARK] ItcMailboxTest test1 " << " took " << duration << " ns\n";
}

TEST_F(ItcMailboxTest, test2)
{
    /***
     * Test scenario: test send receive benchmarking.
     */
    ItcMailbox receiver;
    receiver.setState(true);
    std::chrono::_V2::system_clock::time_point start;
    std::chrono::_V2::system_clock::time_point end;
    
    uint32_t NUMBER_OF_MESSAGES = 1000;
    constexpr uint32_t NUMBER_OF_SENDERS = 10;
    auto sender = [&](uint32_t senderId)
    {
        for(uint32_t i = 0; i < NUMBER_OF_MESSAGES / NUMBER_OF_SENDERS; ++i)
        {
            // std::cout << "ETRUGIA: senderId = " << senderId << ", i = " << i << std::endl;
            auto msg = ItcAdminMessageHelper::allocate(senderId * 10 + i);
            if(!receiver.push(msg))
            {
                break;
            }
        }
    };
    
    start = std::chrono::high_resolution_clock::now();
    std::thread senders[NUMBER_OF_SENDERS];
    for(uint32_t i = 0; i < NUMBER_OF_SENDERS; ++i)
    {
        senders[i] = std::thread(sender, i);
    }
    
    uint32_t count {0};
    while(count < NUMBER_OF_MESSAGES / NUMBER_OF_SENDERS)
    {
        while(!receiver.m_rxMsgQueue->empty())
        {
            // std::cout << "ETRUGIA: count = " << count << std::endl;
            auto msg = receiver.pop(ITC_MODE_RECEIVE_NON_BLOCKING);
            if(msg)
            {
                ItcAdminMessageHelper::deallocate(msg);
                ++count;
            }
        }
    }
    
    for(uint32_t i = 0; i < NUMBER_OF_SENDERS; ++i)
    {
        senders[i].join();
    }
    
    end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "[BENCHMARK] ItcMailboxTest test2 " << " took " << duration / NUMBER_OF_MESSAGES << " ns\n";
}


TEST_F(ItcMailboxTest, test3)
{
    /***
     * Test scenario: test send receive benchmarking.
     */
    ItcMailbox receiver;
    receiver.setState(true);
    std::chrono::_V2::system_clock::time_point start;
    std::chrono::_V2::system_clock::time_point end;
    
    constexpr uint32_t NUMBER_OF_MESSAGES = 1000;
    ItcAdminMessageRawPtr messages[NUMBER_OF_MESSAGES];
    for(uint32_t i = 0; i < NUMBER_OF_MESSAGES; ++i)
    {
        messages[i] = ItcAdminMessageHelper::allocate(i);
    }
    constexpr uint32_t NUMBER_OF_WORKERS = 10;
    auto senderFunc = [&](uint32_t senderId)
    {
        for(uint32_t i = 0; i < NUMBER_OF_MESSAGES / (NUMBER_OF_WORKERS / 2); ++i)
        {
            // std::cout << "ETRUGIA: senderId = " << senderId << ", i = " << i << std::endl;
            if(!receiver.push(messages[(senderId * 10) + i]))
            {
                break;
            }
        }
    };
    
    auto receiverFunc = [&]()
    {
        uint32_t count {0};
        while(count < NUMBER_OF_MESSAGES / (NUMBER_OF_WORKERS / 2))
        {
            auto msg = receiver.pop(ITC_MODE_RECEIVE_NON_BLOCKING);
            if(msg)
            {
                ++count;
            }
        }
    };
    
    std::thread senderThread[NUMBER_OF_WORKERS / 2];
    std::thread receiverThread[NUMBER_OF_WORKERS / 2];
    start = std::chrono::high_resolution_clock::now();
    for(uint32_t i = 0; i < NUMBER_OF_WORKERS / 2; ++i)
    {
        senderThread[i] = std::thread(senderFunc, i);
        receiverThread[i] = std::thread(receiverFunc);
    }
    
    for(uint32_t i = 0; i < NUMBER_OF_WORKERS / 2; ++i)
    {
        senderThread[i].join();
        receiverThread[i].join();
    }
    end = std::chrono::high_resolution_clock::now();
    
    for(uint32_t i = 0; i < NUMBER_OF_MESSAGES; ++i)
    {
        ItcAdminMessageHelper::deallocate(messages[i]);
    }
    
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "[BENCHMARK] ItcMailboxTest test3 " << " took " << duration / NUMBER_OF_MESSAGES << " ns\n";
}

#define FOR_LOOP_ALLOCATION(index) \
    for(uint32_t i = 0; i < NUMBER_OF_MESSAGES / NUMBER_OF_SENDERS; ++i) \
    { \
        messages##index[i] = ItcAdminMessageHelper::allocate(i); \
    }

#define FOR_LOOP_DEALLOCATION(index) \
    for(uint32_t i = 0; i < NUMBER_OF_MESSAGES / NUMBER_OF_SENDERS; ++i) \
    { \
        if(messages##index[i]) \
        { \
            ItcAdminMessageHelper::deallocate(messages##index[i]); \
        } \
    }
    
#define PUSH(index) \
    if(!receiver.push(messages##index[i])) \
    { \
        shouldStop = true; \
    } \
    messages##index[i] = nullptr;
    
TEST_F(ItcMailboxTest, test4)
{
    /***
     * Test scenario: test send receive benchmarking.
     */
    ItcMailbox receiver;
    receiver.setState(true);
    std::chrono::_V2::system_clock::time_point start;
    std::chrono::_V2::system_clock::time_point end;
    
    constexpr uint32_t NUMBER_OF_MESSAGES = 1000;
    constexpr uint32_t NUMBER_OF_SENDERS = 10;
    ItcAdminMessageRawPtr messages0[NUMBER_OF_MESSAGES / NUMBER_OF_SENDERS];
    ItcAdminMessageRawPtr messages1[NUMBER_OF_MESSAGES / NUMBER_OF_SENDERS];
    ItcAdminMessageRawPtr messages2[NUMBER_OF_MESSAGES / NUMBER_OF_SENDERS];
    ItcAdminMessageRawPtr messages3[NUMBER_OF_MESSAGES / NUMBER_OF_SENDERS];
    ItcAdminMessageRawPtr messages4[NUMBER_OF_MESSAGES / NUMBER_OF_SENDERS];
    ItcAdminMessageRawPtr messages5[NUMBER_OF_MESSAGES / NUMBER_OF_SENDERS];
    ItcAdminMessageRawPtr messages6[NUMBER_OF_MESSAGES / NUMBER_OF_SENDERS];
    ItcAdminMessageRawPtr messages7[NUMBER_OF_MESSAGES / NUMBER_OF_SENDERS];
    ItcAdminMessageRawPtr messages8[NUMBER_OF_MESSAGES / NUMBER_OF_SENDERS];
    ItcAdminMessageRawPtr messages9[NUMBER_OF_MESSAGES / NUMBER_OF_SENDERS];
    FOR_LOOP_ALLOCATION(0);
    FOR_LOOP_ALLOCATION(1);
    FOR_LOOP_ALLOCATION(2);
    FOR_LOOP_ALLOCATION(3);
    FOR_LOOP_ALLOCATION(4);
    FOR_LOOP_ALLOCATION(5);
    FOR_LOOP_ALLOCATION(6);
    FOR_LOOP_ALLOCATION(7);
    FOR_LOOP_ALLOCATION(8);
    FOR_LOOP_ALLOCATION(9);
    std::vector<ItcAdminMessageRawPtr> toBeFreeMessages;
    toBeFreeMessages.reserve(NUMBER_OF_MESSAGES);
    auto sender = [&](uint32_t senderId)
    {
        bool shouldStop {false};
        for(uint32_t i = 0; i < NUMBER_OF_MESSAGES / NUMBER_OF_SENDERS; ++i)
        {
            if(shouldStop)
            {
                // std::cout << "ETRUGIA: Sender " << senderId << " stop sending...\n";
                break;
            }
            // std::cout << "ETRUGIA: senderId = " << senderId << ", i = " << i << ", msg = " << std::endl;
            switch (senderId)
            {
            case 0:
                PUSH(0)
                break;
            
            case 1:
                PUSH(1)
                break;
            
            case 2:
                PUSH(2)
                break;
            
            case 3:
                PUSH(3)
                break;
            
            case 4:
                PUSH(4)
                break;
            
            case 5:
                PUSH(5)
                break;
            
            case 6:
                PUSH(6)
                break;
            
            case 7:
                PUSH(7)
                break;
            
            case 8:
                PUSH(8)
                break;
            
            case 9:
                PUSH(9)
                break;
            
            default:
                break;
            }
        }
    };
    
    start = std::chrono::high_resolution_clock::now();
    std::thread senders[NUMBER_OF_SENDERS];
    for(uint32_t i = 0; i < NUMBER_OF_SENDERS; ++i)
    {
        senders[i] = std::thread(sender, i);
    }
    
    uint32_t count {0};
    while(count < NUMBER_OF_MESSAGES)
    {
        // std::cout << "ETRUGIA: count = " << count << std::endl;
        if(count == 100)
        {
            // std::cout << "ETRUGIA: Terminating receiver!" << std::endl;
            std::cout << "ETRUGIA: Queue size before = " << receiver.m_rxMsgQueue->size() << std::endl;
            receiver.setState(false);
            // std::cout << "ETRUGIA: Queue size after = " << receiver.m_rxMsgQueue->size() << std::endl;
            break;
        }
        
        auto msg = receiver.pop(ITC_MODE_RECEIVE_NON_BLOCKING);
        if(msg)
        {
            // std::cout << "ETRUGIA: count = " << count << std::endl;
            toBeFreeMessages.emplace_back(msg);
            ++count;
        }
    }
    
    for(uint32_t i = 0; i < NUMBER_OF_SENDERS; ++i)
    {
        senders[i].join();
    }
    
    end = std::chrono::high_resolution_clock::now();
    
    FOR_LOOP_DEALLOCATION(0)
    FOR_LOOP_DEALLOCATION(1)
    FOR_LOOP_DEALLOCATION(2)
    FOR_LOOP_DEALLOCATION(3)
    FOR_LOOP_DEALLOCATION(4)
    FOR_LOOP_DEALLOCATION(5)
    FOR_LOOP_DEALLOCATION(6)
    FOR_LOOP_DEALLOCATION(7)
    FOR_LOOP_DEALLOCATION(8)
    FOR_LOOP_DEALLOCATION(9)
    
    for(auto &msg : toBeFreeMessages)
    {
        ItcAdminMessageHelper::deallocate(msg);
    }
    
    std::cout << "ETRUGIA: Rx queue size = " << receiver.m_rxMsgQueue->size() << std::endl;
    
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "[BENCHMARK] ItcMailboxTest test4 " << " took " << duration / NUMBER_OF_MESSAGES << " ns\n";
}

} // namespace INTERNAL
} // namespace ITC