#pragma once

#include <string>
#include <memory>
#include <filesystem>

#include <sys/types.h>

// #include <enumUtils.h>

namespace ItcPlatform
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

class FileSystemIf
{
public:
    static std::weak_ptr<FileSystemIf> getInstance();

    FileSystemIf(const FileSystemIf &other) = delete;
    FileSystemIf &operator=(const FileSystemIf &other) = delete;
    FileSystemIf(FileSystemIf &&other) noexcept = delete;
    FileSystemIf &operator=(FileSystemIf &&other) noexcept = delete;
    
	enum class PathType
	{
		UNDEFINED,
		REGULAR_FILE,
		DIRECTORY,
	};
	
	enum class FileSystemIfReturnCode
	{
		ITC_FILESYSTEM_UNDEFINED,
		ITC_FILESYSTEM_OK,
		ITC_FILESYSTEM_SYSCALL_ERROR,
		ITC_FILESYSTEM_INVALID_ARGUMENTS,
		ITC_FILESYSTEM_EXCEPTION_THROWN,
	};
	
    // enum class FileSystemIfReturnCodeRaw
	// {
	// 	ITC_FILESYSTEM_UNDEFINED,
	// 	ITC_FILESYSTEM_OK,
	// 	ITC_FILESYSTEM_SYSCALL_ERROR,
	// };

	// class FileSystemIfReturnCode : public EnumType<FileSystemIfReturnCodeRaw>
	// {
	// public:
	// 	explicit FileSystemIfReturnCode(const FileSystemIfReturnCodeRaw& raw) : EnumType<FileSystemIfReturnCodeRaw>(raw) {}
	// 	explicit FileSystemIfReturnCode() : EnumType<FileSystemIfReturnCodeRaw>(FileSystemIfReturnCodeRaw::ITC_FILESYSTEM_UNDEFINED) {}

	// 	std::string toString() const override
	// 	{
	// 		switch (getRawEnum())
	// 		{
	// 		case FileSystemIfReturnCodeRaw::ITC_FILESYSTEM_UNDEFINED:
	// 			return "ITC_FILESYSTEM_UNDEFINED";

	// 		case FileSystemIfReturnCodeRaw::ITC_FILESYSTEM_OK:
	// 			return "ITC_FILESYSTEM_OK";

	// 		case FileSystemIfReturnCodeRaw::ITC_FILESYSTEM_SYSCALL_ERROR:
	// 			return "ITC_FILESYSTEM_SYSCALL_ERROR";
			
	// 		default:
	// 			return "Unknown EnumType: " + std::to_string(toS32());
	// 		}
	// 	}
	// };
    
	/***
	 * The argument "pos" means, let's say: "itc" directory in "/tmp/itc/abc/log.txt" is 2nd directory
	 * in order of parent-child directories creation (0-indexing),
	 * the 0th directory is "/", the 1st one is "/tmp" and the 2nd one is "/tmp/itc".
	 * 
	 * If:
	 * 	size_t itcDirStartPos = 2;
	 * 	FileSystemIf::getInstance().lock()->createPath("/tmp/itc/abc/def", std::filesystem::perms::all, itcDirStartPos, PathType::DIRECTORY);
	 * 
	 * This itcDirStartPos = 2 means: all directories or files from position 2 is created
	 * with permission 0777. More specifically, those directories "itc", "abc" and file "log.txt" are
	 * under 0777 permission.
	 */
    virtual FileSystemIfReturnCode createPath(const std::filesystem::path &path, std::filesystem::perms mode, size_t pos, PathType type) = 0;
    virtual FileSystemIfReturnCode removePath(const std::filesystem::path &path) = 0;
    virtual bool exists(const std::filesystem::path &path) = 0;
    virtual bool isAccessible(const std::filesystem::path &path) = 0;
    
	
protected:
    FileSystemIf() = default;
    virtual ~FileSystemIf() = default;
}; // class FileSystemIf

} // namespace INTERNAL
} // namespace ItcPlatform