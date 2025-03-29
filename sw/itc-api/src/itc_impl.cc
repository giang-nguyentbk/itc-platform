#pragma once

#include "itc-api/inc/itc_impl.h"

#include <cstdint>

namespace ItcPlatform
{
namespace INTERNAL
{

// using namespace CommonUtils::V1::EnumUtils;
using namespace ItcPlatform::PROVIDED;
using ItcPlatformIfReturnCode = ItcPlatformIf::ItcPlatformIfReturnCode;

/***
 * Just for compilation in unit testing.
 * Those getInstance from <InterfaceClassName>If interface classes
 * must look like this to avoid multiple definition compiler error
 * which is conflicts between this real getInstance implementation
 * and the one is in mock version.
 * 
 * For example, find itcFileSystemIfMock.cc to see mocked implementation
 * of getInstance().
*/
#ifndef UNITTEST
std::weak_ptr<ItcPlatformIf> ItcPlatformIf::getInstance()
{
	return ItcPlatformImpl::getInstance();
}
#endif

std::weak_ptr<ItcPlatformImpl> ItcPlatformImpl::getInstance()
{
    std::scoped_lock<std::mutex> lock(m_singletonMutex);
    if (m_instance == nullptr)
    {
        m_instance.reset(new ItcPlatformImpl);
    }
    return m_instance;
}

ItcPlatformImpl::ItcPlatformImpl()
{
    
}

ItcPlatformImpl::~ItcPlatformImpl()
{
	
}

ItcPlatformIfReturnCode ItcPlatformImpl::initialise(uint32_t mboxCount, uint32_t flags)
{
    return MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_ALREADY_USED);
}

} // namespace INTERNAL
} // namespace ItcPlatform
