#include "itcMutex.h"

#include <gtest/gtest.h>

namespace ITC
{
namespace INTERNAL
{
    
class ItcMutexTest : public testing::Test
{
protected:
    ItcMutexTest()
    {}

    ~ItcMutexTest()
    {}
    
    void SetUp() override
    {}

    void TearDown() override
    {}

protected:
    
};

TEST_F(ItcMutexTest, getTimeDiffInNanoSecTest1)
{
    /***
     * Test scenario: test time diff must be greater than or equal to a sleep time which is 5ms.
     */
    timespec start;
    timespec end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    usleep(5000);
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    long int diff = getTimeDiffInNanoSec(start, end);
    ASSERT_GE(diff, 5000 * 1000);
}


} // namespace INTERNAL
} // namespace ITC