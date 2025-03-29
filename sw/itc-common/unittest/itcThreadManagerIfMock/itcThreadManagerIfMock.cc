#include "itcThreadManagerIfMock.h"

#include <mutex>

namespace ItcPlatform
{
namespace INTERNAL
{
std::shared_ptr<ThreadManagerIfMock> ThreadManagerIfMock::m_instance = nullptr;
std::mutex ThreadManagerIfMock::m_singletonMutex;

std::weak_ptr<ThreadManagerIf> ThreadManagerIf::getInstance()
{
	return ThreadManagerIfMock::getInstance();
}

std::weak_ptr<ThreadManagerIfMock> ThreadManagerIfMock::getInstance()
{
    std::scoped_lock<std::mutex> lock(m_singletonMutex);
    if (m_instance == nullptr)
    {
        m_instance.reset(new ThreadManagerIfMock);
    }
    return m_instance;
}

} // namespace INTERNAL
} // namespace ItcPlatform

