#include "itcSmartContainer.h"

#include <iostream>
#include <memory>
#include <string>
#include <gtest/gtest.h>

#include "itcThreadPool.h"

namespace ITC
{
namespace INTERNAL
{

using namespace ::testing;

struct TestData
{
    std::string name;
    size_t id {0xFFFFFFFF};
    TestData(const std::string &iName, size_t iId = 0xFFFFFFFF)
        : name(iName),
          id(iId)
    {}
    ~TestData() = default;
    
    void reset()
    {
        id = 0xFFFFFFFF;
    }
};

class SmartContainerTest : public testing::Test
{
protected:
    SmartContainerTest()
    {}

    ~SmartContainerTest()
    {}
    
    void SetUp() override
    {
        SmartContainer<TestData>::Callback callback = {
            [](const TestData &data) -> std::string
            {
                return data.name;
            },
            [](TestData &data)
            {
                data.reset();
            }
        };
        m_smartTestDatas = std::make_shared<SmartContainer<TestData>>(callback);
        m_threadPool = std::make_shared<ThreadPool>(5);
    }

    void TearDown() override
    {}
    
protected:
    std::shared_ptr<SmartContainer<TestData>> m_smartTestDatas {nullptr};
    std::function<void(TestData &, size_t)> m_calculateId = [](TestData &data, size_t index)
    {
        data.id &= (index << 16) | (data.id & 0xFFFF);
    };
    std::shared_ptr<ThreadPool> m_threadPool {nullptr};
};

TEST_F(SmartContainerTest, test1)
{
    /***
     * Test scenario: test insert 2 element.
     */
    auto indexOpt = m_smartTestDatas->emplace(TestData("testData1"));
    ASSERT_EQ(indexOpt.has_value(), true);
    ASSERT_EQ(indexOpt.value(), 0);
    m_calculateId(*m_smartTestDatas->m_slotMetadata.at(indexOpt.value()).slot, indexOpt.value());
    indexOpt = m_smartTestDatas->emplace(TestData("testData2"));
    ASSERT_EQ(indexOpt.has_value(), true);
    ASSERT_EQ(indexOpt.value(), 1);
    m_calculateId(*m_smartTestDatas->m_slotMetadata.at(indexOpt.value()).slot, indexOpt.value());
    ASSERT_EQ(m_smartTestDatas->m_slotMetadata.size(), 2);
    ASSERT_EQ(m_smartTestDatas->m_slotMetadata.back().slot->id, 0x0001FFFF);
    ASSERT_EQ(m_smartTestDatas->m_slotMetadata.back().isActive, true);
    ASSERT_EQ(m_smartTestDatas->m_activeSlots.size(), 2);
    ASSERT_EQ(m_smartTestDatas->m_activeSlots.at("testData2"), 1);
    ASSERT_EQ(m_smartTestDatas->m_inactiveSlots.size(), 0);
}

TEST_F(SmartContainerTest, test2)
{
    /***
     * Test scenario: test insert multiple elements.
     */
    auto indexOpt = m_smartTestDatas->emplace(TestData("testData1"));
    ASSERT_EQ(indexOpt.has_value(), true);
    ASSERT_EQ(indexOpt.value(), 0);
    indexOpt = m_smartTestDatas->emplace(TestData("testData2"));
    ASSERT_EQ(indexOpt.has_value(), true);
    ASSERT_EQ(indexOpt.value(), 1);
    indexOpt = m_smartTestDatas->emplace(TestData("testData3"));
    ASSERT_EQ(indexOpt.has_value(), true);
    ASSERT_EQ(indexOpt.value(), 2);
    indexOpt = m_smartTestDatas->emplace(TestData("testData4"));
    ASSERT_EQ(indexOpt.has_value(), true);
    ASSERT_EQ(indexOpt.value(), 3);
    indexOpt = m_smartTestDatas->emplace(TestData("testData5"));
    ASSERT_EQ(indexOpt.has_value(), true);
    ASSERT_EQ(indexOpt.value(), 4);
    ASSERT_EQ(m_smartTestDatas->m_slotMetadata.size(), 5);
    ASSERT_EQ(m_smartTestDatas->m_activeSlots.size(), 5);
    ASSERT_EQ(m_smartTestDatas->m_inactiveSlots.size(), 0);
    for(size_t i = 0; i < 5; ++i)
    {
        ASSERT_EQ(m_smartTestDatas->m_slotMetadata.at(i).isActive, true);
    }
}

TEST_F(SmartContainerTest, test3)
{
    /***
     * Test scenario: test insert an already exist element.
     */
    auto indexOpt = m_smartTestDatas->emplace(TestData("testData1"));
    ASSERT_EQ(indexOpt.has_value(), true);
    ASSERT_EQ(indexOpt.value(), 0);
    indexOpt = m_smartTestDatas->emplace(TestData("testData1"));
    ASSERT_EQ(indexOpt.has_value(), false);
    ASSERT_EQ(m_smartTestDatas->m_slotMetadata.size(), 1);
}

TEST_F(SmartContainerTest, test4)
{
    /***
     * Test scenario: test remove an inserted element.
     */
    auto indexOpt = m_smartTestDatas->emplace(TestData("testData1"));
    ASSERT_EQ(indexOpt.has_value(), true);
    ASSERT_EQ(indexOpt.value(), 0);
    ASSERT_EQ(m_smartTestDatas->m_slotMetadata.size(), 1);
    ASSERT_EQ(m_smartTestDatas->m_slotMetadata.front().isActive, true);
    ASSERT_EQ(m_smartTestDatas->m_activeSlots.size(), 1);
    ASSERT_EQ(m_smartTestDatas->m_activeSlots.at("testData1"), 0);
    ASSERT_EQ(m_smartTestDatas->m_inactiveSlots.size(), 0);
    
    m_smartTestDatas->remove(indexOpt.value());
    ASSERT_EQ(m_smartTestDatas->m_slotMetadata.size(), 1);
    ASSERT_EQ(m_smartTestDatas->m_slotMetadata.front().isActive, false);
    ASSERT_EQ(m_smartTestDatas->m_slotMetadata.front().slot->id, 0xFFFFFFFF);
    ASSERT_EQ(m_smartTestDatas->m_activeSlots.size(), 0);
    ASSERT_EQ(m_smartTestDatas->m_inactiveSlots.size(), 1);
}

TEST_F(SmartContainerTest, test5)
{
    /***
     * Test scenario: test remove an non-existing element.
     */
    auto indexOpt = m_smartTestDatas->emplace(TestData("testData1"));
    ASSERT_EQ(indexOpt.has_value(), true);
    ASSERT_EQ(indexOpt.value(), 0);
    ASSERT_EQ(m_smartTestDatas->m_slotMetadata.size(), 1);
    ASSERT_EQ(m_smartTestDatas->m_slotMetadata.front().isActive, true);
    m_smartTestDatas->remove(indexOpt.value() + 1); // Force wrong index
    ASSERT_EQ(m_smartTestDatas->m_slotMetadata.size(), 1);
    ASSERT_EQ(m_smartTestDatas->m_slotMetadata.front().isActive, true);
}

TEST_F(SmartContainerTest, test6)
{
    /***
     * Test scenario: test remove an element in the middle.
     */
    auto indexOpt = m_smartTestDatas->emplace(TestData("testData1"));
    ASSERT_EQ(indexOpt.has_value(), true);
    ASSERT_EQ(indexOpt.value(), 0);
    indexOpt = m_smartTestDatas->emplace(TestData("testData2"));
    ASSERT_EQ(indexOpt.has_value(), true);
    ASSERT_EQ(indexOpt.value(), 1);
    indexOpt = m_smartTestDatas->emplace(TestData("testData3"));
    ASSERT_EQ(indexOpt.has_value(), true);
    ASSERT_EQ(indexOpt.value(), 2);
    indexOpt = m_smartTestDatas->emplace(TestData("testData4"));
    ASSERT_EQ(indexOpt.has_value(), true);
    ASSERT_EQ(indexOpt.value(), 3);
    indexOpt = m_smartTestDatas->emplace(TestData("testData5"));
    ASSERT_EQ(indexOpt.has_value(), true);
    ASSERT_EQ(indexOpt.value(), 4);
    ASSERT_EQ(m_smartTestDatas->m_slotMetadata.size(), 5);
    ASSERT_EQ(m_smartTestDatas->m_activeSlots.size(), 5);
    ASSERT_EQ(m_smartTestDatas->m_inactiveSlots.size(), 0);
    
    m_smartTestDatas->remove(2);
    ASSERT_EQ(m_smartTestDatas->m_slotMetadata.size(), 5);
    ASSERT_EQ(m_smartTestDatas->m_slotMetadata.at(2).isActive, false);
    ASSERT_EQ(m_smartTestDatas->m_slotMetadata.at(2).slot->id, 0xFFFFFFFF);
    ASSERT_EQ(m_smartTestDatas->m_activeSlots.size(), 4);
    ASSERT_EQ(m_smartTestDatas->m_inactiveSlots.size(), 1);
    ASSERT_EQ(m_smartTestDatas->m_inactiveSlots.front(), 2);
}

TEST_F(SmartContainerTest, test7)
{
    /***
     * Test scenario: test access via index.
     */
    auto indexOpt = m_smartTestDatas->emplace(TestData("testData1"));
    ASSERT_EQ(indexOpt.has_value(), true);
    ASSERT_EQ(indexOpt.value(), 0);
    indexOpt = m_smartTestDatas->emplace(TestData("testData2"));
    ASSERT_EQ(indexOpt.has_value(), true);
    ASSERT_EQ(indexOpt.value(), 1);
    indexOpt = m_smartTestDatas->emplace(TestData("testData3"));
    ASSERT_EQ(indexOpt.has_value(), true);
    ASSERT_EQ(indexOpt.value(), 2);
    ASSERT_EQ(m_smartTestDatas->m_slotMetadata.size(), 3);
    ASSERT_EQ(m_smartTestDatas->m_activeSlots.size(), 3);
    ASSERT_EQ(m_smartTestDatas->m_inactiveSlots.size(), 0);
    
    auto dataOpt = m_smartTestDatas->at(1);
    ASSERT_EQ(dataOpt.has_value(), true);
    auto &data = dataOpt.value().get();
    m_calculateId(data, 1);
    ASSERT_EQ(m_smartTestDatas->m_slotMetadata.at(1).slot->id, 0x0001FFFF);
    ASSERT_EQ(m_smartTestDatas->m_slotMetadata.at(1).slot->name, "testData2");
}

TEST_F(SmartContainerTest, test8)
{
    /***
     * Test scenario: test access via name.
     */
    auto indexOpt = m_smartTestDatas->emplace(TestData("testData1"));
    ASSERT_EQ(indexOpt.has_value(), true);
    ASSERT_EQ(indexOpt.value(), 0);
    indexOpt = m_smartTestDatas->emplace(TestData("testData2"));
    ASSERT_EQ(indexOpt.has_value(), true);
    ASSERT_EQ(indexOpt.value(), 1);
    indexOpt = m_smartTestDatas->emplace(TestData("testData3"));
    ASSERT_EQ(indexOpt.has_value(), true);
    ASSERT_EQ(indexOpt.value(), 2);
    ASSERT_EQ(m_smartTestDatas->m_slotMetadata.size(), 3);
    ASSERT_EQ(m_smartTestDatas->m_activeSlots.size(), 3);
    ASSERT_EQ(m_smartTestDatas->m_inactiveSlots.size(), 0);
    
    auto dataOpt = m_smartTestDatas->at("testData2");
    ASSERT_EQ(dataOpt.has_value(), true);
    auto &data = dataOpt.value().get();
    m_calculateId(data, 1);
    ASSERT_EQ(m_smartTestDatas->m_slotMetadata.at(1).slot->id, 0x0001FFFF);
    ASSERT_EQ(m_smartTestDatas->m_slotMetadata.at(1).slot->name, "testData2");
}

TEST_F(SmartContainerTest, test9)
{
    /***
     * Test scenario: test insert -> remove -> insert (reuse),
     * expect m_activeSlots has only one element of "testData2"
     * no redundant element of "testData1".
     */
    auto indexOpt = m_smartTestDatas->emplace(TestData("testData1"));
    ASSERT_EQ(indexOpt.has_value(), true);
    ASSERT_EQ(indexOpt.value(), 0);
    m_smartTestDatas->remove("testData1");
    ASSERT_EQ(m_smartTestDatas->m_slotMetadata.front().isActive, false);
    ASSERT_EQ(m_smartTestDatas->m_slotMetadata.front().slot->id, 0xFFFFFFFF);
    indexOpt = m_smartTestDatas->emplace(TestData("testData2"));
    ASSERT_EQ(indexOpt.has_value(), true);
    ASSERT_EQ(indexOpt.value(), 0);
    ASSERT_EQ(m_smartTestDatas->m_activeSlots.size(), 1);
    
    auto dataOpt = m_smartTestDatas->at("testData2");
    ASSERT_EQ(dataOpt.has_value(), true);
    ASSERT_EQ(dataOpt.value().get().name, "testData2");
}

// /***
//  * Stress tests
//  */

TEST_F(SmartContainerTest, stressTest1)
{
    /***
     * Test scenario: test multiple threads call insert.
     */
    size_t taskCount = 5;
    size_t insertCount = 10;
    std::vector<std::future<bool>> results;
    for(size_t n = 0; n < taskCount; ++n)
    {
        results.emplace_back(m_threadPool->enqueue(
            [this, n, insertCount]()
            {
                if(n % 2)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                for(size_t i = 0; i < insertCount; ++i)
                {
                    MAYBE_UNUSED auto indexOpt = this->m_smartTestDatas->emplace(TestData("TestData_" + std::to_string(n) + "_" + std::to_string(i)));
                    // std::cout << "ETRUGIA: Inserting TestData_" << n << "_" << i << ", returned index = " << indexOpt.value_or(-1) << ", m_slotMetadata.size = " << this->m_smartTestDatas->m_slotMetadata.size() << ", m_inactiveSlots.size = " << this->m_smartTestDatas->m_inactiveSlots.size() << std::endl;
                }
                return true;
            }
        ));
    }
    
    for(size_t i = 0; i < taskCount; ++i)
    {
        results.at(i).get();
    }
    
    ASSERT_EQ(m_smartTestDatas->m_slotMetadata.size(), taskCount * insertCount);
}

TEST_F(SmartContainerTest, stressTest2)
{
    /***
     * Test scenario: test multiple threads call insert.
     */
    size_t count = 200;
    std::vector<std::future<bool>> results;
    results.emplace_back(
        m_threadPool->enqueue(
            [this, count]()
            {
                for(size_t i = 0; i < count; ++i)
                {
                    MAYBE_UNUSED auto indexOpt = this->m_smartTestDatas->emplace(TestData("TestData_" + std::to_string(i)));
                    // std::cout << "ETRUGIA: Inserted TestData_" << i << ", returned index = " << indexOpt.value_or(-1) << ", m_slotMetadata.size = " << this->m_smartTestDatas->m_slotMetadata.size() << ", m_inactiveSlots.size = " << this->m_smartTestDatas->m_inactiveSlots.size() << std::endl;
                }
                return true;
            }
        )
    );
    
    results.emplace_back(
        m_threadPool->enqueue(
            [this, count]()
            {
                std::this_thread::sleep_for(std::chrono::microseconds(1));
                for(size_t i = 0; i < count; ++i)
                {
                    this->m_smartTestDatas->remove("TestData_" + std::to_string(i));
                    // std::cout << "ETRUGIA: Removed TestData_" << i << ", m_slotMetadata.size = " << this->m_smartTestDatas->m_slotMetadata.size() << ", m_inactiveSlots.size = " << this->m_smartTestDatas->m_inactiveSlots.size() << std::endl;
                }
                return true;
            }
        )
    );
    
    for(size_t i = 0; i < 2; ++i)
    {
        results.at(i).get();
    }
}

TEST_F(SmartContainerTest, stressTest3)
{
    /***
     * Test scenario: test multiple threads call insert and remove, expectation is:
     * + No active slot eventually (If no race condition, active slots that were used are cleanup properly).
     */
    size_t repeat = 200;
    size_t taskCount = 5;
    std::vector<std::future<bool>> results;
    for(size_t n = 0; n < taskCount; ++n)
    {
        results.emplace_back(
            m_threadPool->enqueue(
                [this, n, repeat]()
                {
                    for(size_t i = 0; i < repeat; ++i)
                    {
                        MAYBE_UNUSED auto indexOpt = this->m_smartTestDatas->emplace(TestData("TestData_" + std::to_string(n) + "_" + std::to_string(i)));
                        // std::cout << "ETRUGIA: Inserted TestData_" << n << "_" << i << ", index = " << indexOpt.value_or(-1) << ", m_slotMetadata.size = " << this->m_smartTestDatas->m_slotMetadata.size() << ", m_inactiveSlots.size = " << this->m_smartTestDatas->m_inactiveSlots.size() << std::endl;
                        this->m_smartTestDatas->remove(indexOpt.value_or(0xFFFFFFFF));
                        // std::cout << "ETRUGIA: Removed TestData_" << n << "_" << i << ", index = " << index << ", m_slotMetadata.size = " << this->m_smartTestDatas->m_slotMetadata.size() << ", m_inactiveSlots.size = " << this->m_smartTestDatas->m_inactiveSlots.size() << std::endl;
                    }
                    return true;
                }
            )
        );
    }
    
    for(size_t i = 0; i < taskCount; ++i)
    {
        results.at(i).get();
    }
    
    ASSERT_EQ(m_smartTestDatas->m_activeSlots.size(), 0);
}

TEST_F(SmartContainerTest, stressTest4)
{
    /***
     * Test scenario: test multiple threads call insert, remove, expectation is:
     * + No slot left at all (If no race condition, all slots that were used are removed entirely).
     */
    size_t repeat = 200;
    size_t taskCount = 2;
    std::vector<std::future<bool>> results;
    for(size_t n = 0; n < taskCount; ++n)
    {
        results.emplace_back(
            m_threadPool->enqueue(
                [this, n, repeat]()
                {
                    size_t counterpartIndex = n == 0 ? 1 : 0;
                    for(size_t i = 0; i < repeat; ++i)
                    {
                        MAYBE_UNUSED auto indexOpt = this->m_smartTestDatas->emplace(TestData("TestData_" + std::to_string(n) + "_" + std::to_string(i)));
                        // std::cout << "ETRUGIA: Inserted TestData_" << n << "_" << i << ", index = " << indexOpt.value_or(-1) << ", m_slotMetadata.size = " << this->m_smartTestDatas->m_slotMetadata.size() << ", m_activeSlots.size = " << this->m_smartTestDatas->m_activeSlots.size() << std::endl;
                        auto dataOpt = m_smartTestDatas->at("TestData_" + std::to_string(counterpartIndex) + "_" + std::to_string(i));
                        if(dataOpt.has_value())
                        {
                            auto &data = dataOpt.value().get();
                            data.id = counterpartIndex * i;
                        }
                        this->m_smartTestDatas->remove(indexOpt.value_or(0xFFFFFFFF));
                        // std::cout << "ETRUGIA: Removed TestData_" << n << "_" << i << ", index = " << indexOpt.value_or(-1) << ", m_slotMetadata.size = " << this->m_smartTestDatas->m_slotMetadata.size() << ", m_activeSlots.size = " << this->m_smartTestDatas->m_activeSlots.size() << std::endl;
                    }
                    return true;
                }
            )
        );
    }
    
    for(size_t i = 0; i < taskCount; ++i)
    {
        results.at(i).get();
    }
    
    ASSERT_EQ(m_smartTestDatas->m_activeSlots.size(), 0);
}

TEST_F(SmartContainerTest, stressTest5)
{
    /***
     * Test scenario: test multiple threads call insert, remove, expectation is:
     * + No slot left at all (If no race condition, all slots that were used are removed entirely).
     */
    size_t repeat = 200;
    size_t taskCount = 2;
    std::vector<std::future<bool>> results;
    for(size_t n = 0; n < taskCount; ++n)
    {
        results.emplace_back(
            m_threadPool->enqueue(
                [this, n, repeat]()
                {
                    size_t counterpartIndex = n == 0 ? 1 : 0;
                    for(size_t i = 0; i < repeat; ++i)
                    {
                        MAYBE_UNUSED auto indexOpt = this->m_smartTestDatas->emplace(TestData("TestData_" + std::to_string(n) + "_" + std::to_string(i)));
                        // std::cout << "ETRUGIA: Inserted TestData_" << n << "_" << i << ", index = " << indexOpt.value_or(-1) << ", m_slotMetadata.size = " << this->m_smartTestDatas->m_slotMetadata.size() << ", m_activeSlots.size = " << this->m_smartTestDatas->m_activeSlots.size() << std::endl;
                        auto dataOpt = m_smartTestDatas->at("TestData_" + std::to_string(counterpartIndex) + "_" + std::to_string(i));
                        if(dataOpt.has_value())
                        {
                            auto &data = dataOpt.value().get();
                            data.id = counterpartIndex * i;
                        }
                        m_smartTestDatas->size();
                        m_smartTestDatas->capacity();
                        this->m_smartTestDatas->remove(indexOpt.value_or(0xFFFFFFFF));
                        // std::cout << "ETRUGIA: Removed TestData_" << n << "_" << i << ", index = " << indexOpt.value_or(-1) << ", m_slotMetadata.size = " << this->m_smartTestDatas->m_slotMetadata.size() << ", m_activeSlots.size = " << this->m_smartTestDatas->m_activeSlots.size() << std::endl;
                        if((n == 0 && i % 20 == 0) || (n == 1 && i % 30 == 0))
                        {
                            m_smartTestDatas->clear();
                        }
                    }
                    return true;
                }
            )
        );
    }
    
    for(size_t i = 0; i < taskCount; ++i)
    {
        results.at(i).get();
    }
    
    ASSERT_EQ(m_smartTestDatas->m_activeSlots.size(), 0);
    
    // size_t idx = 0;
    // for(auto &md : m_smartTestDatas->m_slotMetadata)
    // {
    //     std::cout << "ETRUGIA: m_slotMetadata index = " << idx << ", isActive = " << md.isActive << std::endl;
    //     ++idx;
    // }
    
    // while(!m_smartTestDatas->m_inactiveSlots.empty())
    // {
    //     std::cout << "ETRUGIA: m_inactiveSlots index = " << m_smartTestDatas->m_inactiveSlots.front() << std::endl;
    //     m_smartTestDatas->m_inactiveSlots.pop();
    // }
    
    // for(auto &activeSlot : m_smartTestDatas->m_activeSlots)
    // {
    //     std::cout << "ETRUGIA: m_activeSlots key = " << activeSlot.first << ", index = " << activeSlot.second << std::endl;
    // }
}

} // namespace INTERNAL
} // namespace ITC