#include "itcLockFreeHashMap.h"

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

size_t constexpr BUCKET_SIZE = 5120;
size_t constexpr NODE_PER_BUCKET_SIZE = 16;

class LockFreeHashMapTest : public testing::Test
{
protected:
    LockFreeHashMapTest()
    {}

    ~LockFreeHashMapTest()
    {}
    
    void SetUp() override
    {
        m_hashMap = std::make_shared<LockFreeHashMap<std::string, size_t, BUCKET_SIZE, NODE_PER_BUCKET_SIZE>>();
    }

    void TearDown() override
    {}
    
    void benchmarkSPSC()
    {
        std::chrono::_V2::system_clock::time_point start;
        std::chrono::_V2::system_clock::time_point end;
        
        long long NUMBER_OF_ENTRIES = 1000;
        // Producer thread: Enqueue items
        auto producer = [&]() {
            for (uint32_t i = 0; i < NUMBER_OF_ENTRIES; ++i) {
                auto success = m_hashMap->insert(std::to_string(i), i); // Add items to the queue
                // std::cout << "ETRUGIA: Produced: " << i << std::endl;
                if(!success)
                {
                    std::cout << "ETRUGIA: Failed to insert!" << std::endl;
                    break;
                }
            }
        };

        // Consumer thread: Dequeue items
        auto consumer = [&]() {
            long long count = 0;
            while(count < NUMBER_OF_ENTRIES)
            {
                // std::cout << "ETRUGIA: Count: " << count << std::endl;
                if(auto it = m_hashMap->find(std::to_string(count)); it.has_value())
                {
                    // std::cout << "ETRUGIA: Consumed: " << it.value() << std::endl;
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
        m_hashMap->clear();

        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        std::cout << "[BENCHMARK] LockFreeHashMap " << " took " << duration / NUMBER_OF_ENTRIES << " ns\n";
    }
    
    void benchmarkMPMC()
    {
        std::chrono::_V2::system_clock::time_point start;
        std::chrono::_V2::system_clock::time_point end;
        
        uint64_t NUMBER_OF_ENTRIES = 4000;
        constexpr uint64_t NUMBER_OF_WORKERS = 8;
        // Producer thread: Enqueue items
        auto producer = [&]() {
            for (uint64_t i = 0; i < NUMBER_OF_ENTRIES / NUMBER_OF_WORKERS; ++i) {
                m_hashMap->insert(std::to_string(i), i); // Add items to the queue
                // std::cout << "Produced: " << i << std::endl;
            }
        };

        // Consumer thread: Dequeue items
        auto consumer = [&]() {
            uint64_t count = 0;
            while(count < NUMBER_OF_ENTRIES / NUMBER_OF_WORKERS)
            {
                if(auto it = m_hashMap->find(std::to_string(count)); it.has_value())
                {
                    // Wait until an item is available
                    // std::cout << "Consumed: " << value << ", count = " << count << std::endl;
                    ++count;
                }
            }
        };

        std::thread producerThread[NUMBER_OF_WORKERS/2];
        std::thread consumerThread[NUMBER_OF_WORKERS/2];
        start = std::chrono::high_resolution_clock::now();
        // Create threads for producer and consumer
        for(uint64_t i = 0; i < NUMBER_OF_WORKERS / 2; ++i)
        {
            producerThread[i] = std::thread(producer);
        }
        for(uint64_t i = 0; i < NUMBER_OF_WORKERS / 2; ++i)
        {
            consumerThread[i] = std::thread(consumer);
        }

        // Wait for threads to finish
        for(uint64_t i = 0; i < NUMBER_OF_WORKERS / 2; ++i)
        {
            producerThread[i].join();
            consumerThread[i].join();
        }
        end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        std::cout << "[BENCHMARK] LockFreeHashMap " << " took " << duration / NUMBER_OF_ENTRIES << " ns\n";
        m_hashMap->clear();
    }
    
    void benchmarkMPMCRemove()
    {
        std::chrono::_V2::system_clock::time_point start;
        std::chrono::_V2::system_clock::time_point end;
        
        uint64_t NUMBER_OF_ENTRIES = 4000;
        constexpr uint64_t NUMBER_OF_WORKERS = 8;
        // Producer thread: Enqueue items
        auto producer = [&]() {
            for (uint64_t i = 0; i < NUMBER_OF_ENTRIES / NUMBER_OF_WORKERS; ++i) {
                m_hashMap->insert(std::to_string(i), i); // Add items to the queue
                // std::cout << "Produced: " << i << std::endl;
            }
        };

        // Consumer thread: Dequeue items
        auto consumer = [&]() {
            uint64_t count = 0;
            while(count < NUMBER_OF_ENTRIES / NUMBER_OF_WORKERS)
            {
                if(m_hashMap->remove(std::to_string(count)))
                {
                    // Wait until an item is available
                    // std::cout << "Consumed: " << value << ", count = " << count << std::endl;
                    ++count;
                }
            }
        };

        std::thread producerThread[NUMBER_OF_WORKERS/2];
        std::thread consumerThread[NUMBER_OF_WORKERS/2];
        start = std::chrono::high_resolution_clock::now();
        // Create threads for producer and consumer
        for(uint64_t i = 0; i < NUMBER_OF_WORKERS / 2; ++i)
        {
            producerThread[i] = std::thread(producer);
        }
        for(uint64_t i = 0; i < NUMBER_OF_WORKERS / 2; ++i)
        {
            consumerThread[i] = std::thread(consumer);
        }

        // Wait for threads to finish
        for(uint64_t i = 0; i < NUMBER_OF_WORKERS / 2; ++i)
        {
            producerThread[i].join();
            consumerThread[i].join();
        }
        end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        std::cout << "[BENCHMARK] LockFreeHashMap " << " took " << duration / NUMBER_OF_ENTRIES << " ns\n";
        for(auto &bucket : m_hashMap->buckets)
        {
            for(auto &entry : bucket.entries)
            {
                ASSERT_EQ(entry.used, false);
            }
        }
    }

protected:
    std::shared_ptr<LockFreeHashMap<std::string, size_t, BUCKET_SIZE, NODE_PER_BUCKET_SIZE>> m_hashMap {nullptr};
};

TEST_F(LockFreeHashMapTest, test1)
{
    /***
     * Test scenario: test Single Producer Single Consumer hash map with find.
     */
    benchmarkSPSC();
}

TEST_F(LockFreeHashMapTest, test2)
{
    /***
     * Test scenario: test Multi Producer Multi Consumer hash map with find.
     */
    benchmarkMPMC();
}

TEST_F(LockFreeHashMapTest, test3)
{
    /***
     * Test scenario: test Multi Producer Multi Consumer hash map with remove.
     */
    benchmarkMPMCRemove();
}


} // namespace INTERNAL
} // namespace ITC