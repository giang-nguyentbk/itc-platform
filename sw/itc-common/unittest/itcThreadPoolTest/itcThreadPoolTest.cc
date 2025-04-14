#include "itcThreadPool.h"

#include <iostream>
#include <memory>
#include <string>
#include <gtest/gtest.h>

namespace ITC
{
namespace INTERNAL
{

using namespace ::testing;

class ThreadPoolTest : public testing::Test
{
protected:
    ThreadPoolTest()
    {}

    ~ThreadPoolTest()
    {}
    
    void SetUp() override
    {
        m_threadPool = std::make_shared<ThreadPool>(5);
    }

    void TearDown() override
    {}
    
protected:
    std::shared_ptr<ThreadPool> m_threadPool;
};

TEST_F(ThreadPoolTest, test1)
{
    /***
     * Test scenario: test enqueue 10 tasks and check results.
     */
    std::vector<std::future<int>> results;
    
    for(int i = 0; i < 10; ++i)
    {
        results.emplace_back(
            m_threadPool->enqueue([i]()
            {
                return i*i;
            })
        );
    }

    for(int i = 0; i < 10; ++i)
    {
        ASSERT_EQ(results.at(i).get(), i*i);
    }
}


} // namespace INTERNAL
} // namespace ITC