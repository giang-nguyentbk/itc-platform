#include "itcConcurrentContainer.h"

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

class ConcurrentContainerTest : public testing::Test
{
protected:
    ConcurrentContainerTest()
    {}

    ~ConcurrentContainerTest()
    {}
    
    void SetUp() override
    {}

    void TearDown() override
    {}
    
    void benchmarkSPSC()
    {
        std::chrono::_V2::system_clock::time_point start;
        std::chrono::_V2::system_clock::time_point end;
        
        constexpr uint32_t NUMBER_OF_ENTRIES = 1000;
        // Producer thread: Enqueue items
        auto producer = [&]() {
            for (uint32_t i = 0; i < NUMBER_OF_ENTRIES; ++i) {
                auto index = m_cContainer.insert(std::to_string(i), std::move(i)); // Add items to the queue
                // std::cout << "ETRUGIA: Produced: " << i << std::endl;
                if(index == INVALID_INDEX)
                {
                    std::cout << "ETRUGIA: Failed to insert!" << std::endl;
                    break;
                }
            }
        };

        // Consumer thread: Dequeue items
        auto consumer = [&]() {
            uint32_t count = 0;
            while(count < NUMBER_OF_ENTRIES)
            {
                // std::cout << "ETRUGIA: Count: " << count << std::endl;
                if(m_cContainer.recycle(std::to_string(count)))
                {
                    ++count;
                }
            }
            // std::cout << "ETRUGIA: Exchanged messages count: " << count << std::endl;
        };

        start = std::chrono::high_resolution_clock::now();
        // Create threads for producer and consumer
        std::thread producerThread(producer);
        std::thread consumerThread(consumer);

        // Wait for threads to finish
        producerThread.join();
        consumerThread.join();
        end = std::chrono::high_resolution_clock::now();
        ASSERT_EQ(m_cContainer.size(), 0);

        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        std::cout << "[BENCHMARK] ConcurrentContainer " << " took " << duration / NUMBER_OF_ENTRIES << " ns\n";
    }
    
    void benchmark2()
    {
        std::chrono::_V2::system_clock::time_point start;
        std::chrono::_V2::system_clock::time_point end;
        
        constexpr uint32_t NUMBER_OF_ENTRIES = 1000;
        constexpr uint32_t NUMBER_OF_WORKERS = 10;
        // Producer thread: Enqueue items
        auto worker = [&](uint32_t workerId) {
            for (uint32_t i = 0; i < NUMBER_OF_ENTRIES / NUMBER_OF_WORKERS; ++i) {
                // std::cout << "ETRUGIA: workerId = " << workerId << ", i = " << i << std::endl;
                auto index = m_cContainer.insert(std::to_string(workerId * 10 + i), std::move(workerId * 10 + i)); // Add items to the queue
                // std::cout << "ETRUGIA: Produced: " << i << std::endl;
                if(index == INVALID_INDEX)
                {
                    std::cout << "ETRUGIA: Failed to insert, i = " << workerId * 10 + i << std::endl;
                    break;
                }
                if(!m_cContainer.recycle(std::to_string(workerId * 10 + i)))
                {
                    std::cout << "ETRUGIA: Failed to recycle, index = " << index << std::endl;
                    break;
                }
            }
        };

        std::thread workers[NUMBER_OF_WORKERS];
        // Create threads for producer and consumer
        start = std::chrono::high_resolution_clock::now();
        for(uint32_t i = 0; i < NUMBER_OF_WORKERS; ++i)
        {
            workers[i] = std::thread(worker, i);
        }

        // Wait for threads to finish
        for(uint32_t i = 0; i < NUMBER_OF_WORKERS; ++i)
        {
            workers[i].join();
        }
        end = std::chrono::high_resolution_clock::now();
        ASSERT_EQ(m_cContainer.size(), 0);

        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        std::cout << "[BENCHMARK] ConcurrentContainer " << " took " << duration / NUMBER_OF_ENTRIES << " ns\n";
    }
    
protected:
    ConcurrentContainer<uint32_t, 1024> m_cContainer {0};
};

TEST_F(ConcurrentContainerTest, test1)
{
    /***
     * Test scenario: test insert 2 element.
     */
    benchmarkSPSC();
}

TEST_F(ConcurrentContainerTest, test2)
{
    /***
     * Test scenario: test insert 2 element.
     */
    benchmark2();
}


} // namespace INTERNAL
} // namespace ITC