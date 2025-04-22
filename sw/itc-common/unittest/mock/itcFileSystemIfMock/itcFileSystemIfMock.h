#pragma once

#include "itcFileSystemIf.h"
#include "itcConstant.h"

#include <gmock/gmock.h>

namespace ITC
{
namespace INTERNAL
{
using FileSystemIfReturnCode = FileSystemIf::FileSystemIfReturnCode;

class FileSystemIfMock : public FileSystemIf
{
public:
    static std::weak_ptr<FileSystemIfMock> getInstance();
    virtual ~FileSystemIfMock() = default;
    
    MOCK_METHOD(FileSystemIfReturnCode, createPath, (const std::filesystem::path &path, PathType type, size_t pos, std::filesystem::perms mode), (override));
    MOCK_METHOD(FileSystemIfReturnCode, removePath, (const std::filesystem::path &path), (override));
    MOCK_METHOD(bool, exists, (const std::filesystem::path &path), (override));
    MOCK_METHOD(bool, isAccessible, (const std::filesystem::path &path), (override));

private:
    FileSystemIfMock() = default;

private:
    SINGLETON_DECLARATION(FileSystemIfMock)
    
    friend class ItcTransportLSocketTest;
    friend class ItcTransportSysvMsgQueueTest;
}; // class FileSystemIfMock



} // namespace INTERNAL
} // namespace ITC