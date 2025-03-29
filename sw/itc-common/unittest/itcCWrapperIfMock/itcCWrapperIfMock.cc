#include "itcCWrapperIfMock.h"

#include <mutex>

namespace ItcPlatform
{
namespace INTERNAL
{

std::shared_ptr<CWrapperIfMock> CWrapperIfMock::m_instance = nullptr;
std::mutex CWrapperIfMock::m_singletonMutex;

std::weak_ptr<CWrapperIf> CWrapperIf::getInstance()
{
	return CWrapperIfMock::getInstance();
}

std::weak_ptr<CWrapperIfMock> CWrapperIfMock::getInstance()
{
    std::scoped_lock<std::mutex> lock(m_singletonMutex);
    if (m_instance == nullptr)
    {
        m_instance.reset(new CWrapperIfMock);
    }
    return m_instance;
}

} // namespace INTERNAL
} // namespace ItcPlatform

