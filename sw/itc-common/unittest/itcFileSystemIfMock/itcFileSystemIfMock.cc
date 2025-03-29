#include "itcFileSystemIfMock.h"

#include <mutex>

namespace ItcPlatform
{
namespace INTERNAL
{
std::shared_ptr<FileSystemIfMock> FileSystemIfMock::m_instance = nullptr;
std::mutex FileSystemIfMock::m_singletonMutex;

std::weak_ptr<FileSystemIf> FileSystemIf::getInstance()
{
	return FileSystemIfMock::getInstance();
}

std::weak_ptr<FileSystemIfMock> FileSystemIfMock::getInstance()
{
    std::scoped_lock<std::mutex> lock(m_singletonMutex);
    if (m_instance == nullptr)
    {
        m_instance.reset(new FileSystemIfMock);
    }
    return m_instance;
}

} // namespace INTERNAL
} // namespace ItcPlatform

