#include "itcPlatformIfMock.h"

#include <mutex>

namespace ItcPlatform
{
namespace PROVIDED
{
std::weak_ptr<ItcPlatformIf> ItcPlatformIf::getInstance()
{
	return std::static_pointer_cast<ItcPlatformIf>(INTERNAL::ItcPlatformIfMock::getInstance().lock());
}
}

namespace INTERNAL
{
std::shared_ptr<ItcPlatformIfMock> ItcPlatformIfMock::m_instance = nullptr;
std::mutex ItcPlatformIfMock::m_singletonMutex;

std::weak_ptr<ItcPlatformIfMock> ItcPlatformIfMock::getInstance()
{
    std::scoped_lock<std::mutex> lock(m_singletonMutex);
    if (m_instance == nullptr)
    {
        m_instance.reset(new ItcPlatformIfMock);
    }
    return m_instance;
}

} // namespace INTERNAL
} // namespace ItcPlatform

