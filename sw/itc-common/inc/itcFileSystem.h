#pragma once

#include <mutex>

#include "itcFileSystemIf.h"

namespace ItcPlatform
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

class FileSystem : public FileSystemIf
{
public:
    static std::weak_ptr<FileSystem> getInstance();
	virtual ~FileSystem() = default;

    
    // First prevent copy/move construtors/assignments
	FileSystem(const FileSystem&)               = delete;
	FileSystem(FileSystem&&)                    = delete;
	FileSystem& operator=(const FileSystem&)    = delete;
	FileSystem& operator=(FileSystem&&)         = delete;

    FileSystemIfReturnCode createPath(const std::filesystem::path &path, std::filesystem::perms mode, size_t pos, PathType type) override;
    FileSystemIfReturnCode removePath(const std::filesystem::path &path) override;
    bool exists(const std::filesystem::path &path) override;
    bool isAccessible(const std::filesystem::path &path) override;

private:
    FileSystem() = default;

private:
    static std::shared_ptr<FileSystem> m_instance;
	static std::mutex m_singletonMutex;
    
    friend class FileSystemIfTest;

}; // class FileSystem

} // namespace INTERNAL
} // namespace ItcPlatform