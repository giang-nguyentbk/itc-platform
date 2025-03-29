#pragma once

#include "itcFileSystemIf.h"

#include <gmock/gmock.h>

namespace ItcPlatform
{
namespace INTERNAL
{
using FileSystemIfReturnCode = FileSystemIf::FileSystemIfReturnCode;

class FileSystemIfMock : public FileSystemIf
{
public:
    static std::weak_ptr<FileSystemIfMock> getInstance();
    virtual ~FileSystemIfMock() = default;
    
    MOCK_METHOD(FileSystemIfReturnCode, createPath, (const std::filesystem::path &path, std::filesystem::perms mode, size_t pos, PathType type), (override));
    MOCK_METHOD(FileSystemIfReturnCode, removePath, (const std::filesystem::path &path), (override));
    MOCK_METHOD(bool, exists, (const std::filesystem::path &path), (override));
    MOCK_METHOD(bool, isAccessible, (const std::filesystem::path &path), (override));

private:
    FileSystemIfMock() = default;

private:
    static std::shared_ptr<FileSystemIfMock> m_instance;
	static std::mutex m_singletonMutex;
    
    friend class ItcTransportLSocketTest;
    friend class ItcTransportSysvMsgQueueTest;
}; // class FileSystemIfMock



} // namespace INTERNAL
} // namespace ItcPlatform