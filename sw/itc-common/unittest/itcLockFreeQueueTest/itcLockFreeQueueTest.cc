#include "itcLockFreeQueue.h"

#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <gtest/gtest.h>


namespace ITC
{
namespace INTERNAL
{

using namespace ::testing;

size_t constexpr QUEUE_SIZE = 1048576;

class LockFreeQueueTest : public testing::Test
{
protected:
    LockFreeQueueTest()
    {}

    ~LockFreeQueueTest()
    {}
    
    void SetUp() override
    {
        m_spscQueue = std::make_shared<LockFreeQueue<uint32_t, QUEUE_SIZE, 0, true, true, false, true>>();
        m_mpmcQueue = std::make_shared<LockFreeQueue<uint32_t, QUEUE_SIZE, 0, true, true, false, false>>();
    }

    void TearDown() override
    {}
    
    void benchmarkSPSC()
    {
        std::chrono::_V2::system_clock::time_point start;
        std::chrono::_V2::system_clock::time_point end;
        
        long long NUMBER_OF_MESSAGES = QUEUE_SIZE;
        // Producer thread: Enqueue items
        auto producer = [this, NUMBER_OF_MESSAGES]() {
            for (uint32_t i = 0; i < NUMBER_OF_MESSAGES; ++i) {
                this->m_spscQueue->tryPush(i + 1); // Add items to the queue
                // std::cout << "Produced: " << i << std::endl;
            }
        };

        // Consumer thread: Dequeue items
        auto consumer = [this, NUMBER_OF_MESSAGES]() {
            long long count = 0;
            while(count < NUMBER_OF_MESSAGES)
            {
                [[maybe_unused]] uint32_t value;
                if(this->m_spscQueue->tryPop(value))
                {
                    // Wait until an item is available
                    // std::cout << "Consumed: " << value << ", count = " << count << std::endl;
                    ++count;
                }
            }
            // std::cout << "ETRUGIA: Exchanged messages count: " << count << std::endl;
            ASSERT_EQ(count, NUMBER_OF_MESSAGES);
        };

        start = std::chrono::high_resolution_clock::now();
        // Create threads for producer and consumer
        std::thread producerThread(producer);
        std::thread consumerThread(consumer);

        // Wait for threads to finish
        producerThread.join();
        consumerThread.join();
        end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        std::cout << "[BENCHMARK] LockFreeQueue test1 " << " took " << duration / NUMBER_OF_MESSAGES << " ns\n";
    }
    
    void benchmarkMPMC()
    {
        std::chrono::_V2::system_clock::time_point start;
        std::chrono::_V2::system_clock::time_point end;
        
        uint32_t NUMBER_OF_MESSAGES = QUEUE_SIZE;
        constexpr uint32_t NUMBER_OF_PRODUCERS = 4;
        constexpr uint32_t NUMBER_OF_CONSUMERS = 4;
        // Producer thread: Enqueue items
        auto producer = [&]() {
            for (uint32_t i = 0; i < NUMBER_OF_MESSAGES / NUMBER_OF_PRODUCERS; ++i) {
                this->m_mpmcQueue->tryPush(i + 1); // Add items to the queue
                // std::cout << "Produced: " << i << std::endl;
            }
        };

        // Consumer thread: Dequeue items
        auto consumer = [&]() {
            long long count = 0;
            while(count < NUMBER_OF_MESSAGES / NUMBER_OF_CONSUMERS)
            {
                [[maybe_unused]] uint32_t value;
                if(this->m_mpmcQueue->tryPop(value))
                {
                    // Wait until an item is available
                    // std::cout << "Consumed: " << value << ", count = " << count << std::endl;
                    ++count;
                }
            }
            ASSERT_EQ(count, NUMBER_OF_MESSAGES / NUMBER_OF_CONSUMERS);
        };

        std::thread producerThread[NUMBER_OF_PRODUCERS];
        std::thread consumerThread[NUMBER_OF_CONSUMERS];
        start = std::chrono::high_resolution_clock::now();
        // Create threads for producer and consumer
        for(uint32_t i = 0; i < NUMBER_OF_PRODUCERS; ++i)
        {
            producerThread[i] = std::thread(producer);
            consumerThread[i] = std::thread(consumer);
        }

        // Wait for threads to finish
        for(uint32_t i = 0; i < NUMBER_OF_PRODUCERS; ++i)
        {
            producerThread[i].join();
            consumerThread[i].join();
        }
        end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        std::cout << "[BENCHMARK] LockFreeQueue test2 " << " took " << duration / NUMBER_OF_MESSAGES << " ns\n";
    }

protected:
    std::shared_ptr<LockFreeQueue<uint32_t, QUEUE_SIZE, 0, true, true, false, true>> m_spscQueue {nullptr};
    std::shared_ptr<LockFreeQueue<uint32_t, QUEUE_SIZE, 0, true, true, false, false>> m_mpmcQueue {nullptr};
};

TEST_F(LockFreeQueueTest, test1)
{
    /***
     * Test scenario: test Single Producer Single Consumer queue.
     */
    benchmarkSPSC();
}

TEST_F(LockFreeQueueTest, test2)
{
    /***
     * Test scenario: test Single Producer Single Consumer queue.
     */
    benchmarkMPMC();
}


} // namespace INTERNAL
} // namespace ITC