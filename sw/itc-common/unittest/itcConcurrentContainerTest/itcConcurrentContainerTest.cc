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

struct TestData
{
    std::string name;
    uint32_t id;
};

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
    
    void testRaceCondtition()
    {
        constexpr uint32_t NUMBER_OF_MODIFIERS = 100;
        auto modifier = [&](uint32_t modiferId) {
            // std::cout << "ETRUGIA: ModifierId = " << modiferId << ", start modifying...\n";
            TestData *entry = m_cContainer.tryPopFromQueue();
            m_cContainer.addEntryToHashMap(entry->name, entry);
            m_cContainer.removeEntryFromHashMap(entry->name);
            entry->name = "default";
            m_cContainer.tryPushIntoQueue(entry);
            // std::cout << "ETRUGIA: ModifierId = " << modiferId << ", finish modifying...\n";
        };

        std::thread modifierThreads[NUMBER_OF_MODIFIERS];
        for(uint32_t i = 0; i < NUMBER_OF_MODIFIERS; ++i)
        {
            modifierThreads[i] = std::thread(modifier, i);
        }
        
        for(uint32_t i = 0; i < NUMBER_OF_MODIFIERS; ++i)
        {
            modifierThreads[i].join();
        }
        
        ASSERT_EQ(m_cContainer.size(), 0);
    }
    
protected:
    ConcurrentContainer<TestData, 64> m_cContainer {[](TestData *entry, uint32_t index)
    {
        entry->name = "TestData";
        entry->name += std::to_string(index);
        entry->id = index;
    }};
};

TEST_F(ConcurrentContainerTest, test1)
{
    /***
     * Test scenario: test insert 2 element.
     */
    testRaceCondtition();
}

} // namespace INTERNAL
} // namespace ITC