#include "itcPlatform.h"

#include <iostream>
#include <memory>
#include <string>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

#include "itc.h"
#include "itcConstant.h"
#include "itcFileSystemIfMock.h"

namespace ITC
{
namespace INTERNAL
{
using namespace ::testing;
using namespace ITC::PROVIDED;
using ItcPlatformIfReturnCode = ItcPlatformIf::ItcPlatformIfReturnCode;

class ItcPlatformIfTest : public testing::Test
{
protected:
    ItcPlatformIfTest()
    {}

    ~ItcPlatformIfTest()
    {}
    
    void SetUp() override
    {
        m_itcPlatform = ItcPlatform::getInstance().lock();
    }

    void TearDown() override
    {
        // m_itcPlatform->m_instance.reset();
    }
    
    
protected:
    std::shared_ptr<ItcPlatform> m_itcPlatform;
};

TEST_F(ItcPlatformIfTest, checkAndStartItcServerTest1)
{
    /***
     * Test scenario: test start daemon successfully.
     */
    auto created = m_itcPlatform->checkAndStartItcServer();
    ASSERT_EQ(created, true);
    m_itcPlatform->m_instance.reset();
}



} // namespace INTERNAL
} // namespace ITC