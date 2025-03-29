#include "itcFileSystem.h"

#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <mutex>

#include <cstdint>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

// #include <traceIf.h>
// #include "itc-common/inc/itcTptProvider.h"
#include "itc.h"
#include "itcCWrapperIf.h"

namespace ItcPlatform
{
namespace INTERNAL
{

// using namespace CommonUtils::V1::EnumUtils;
using namespace ItcPlatform::PROVIDED;
using FileSystemIfReturnCode = FileSystemIf::FileSystemIfReturnCode;

std::shared_ptr<FileSystem> FileSystem::m_instance = nullptr;
std::mutex FileSystem::m_singletonMutex;

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
std::weak_ptr<FileSystemIf> FileSystemIf::getInstance()
{
	return FileSystem::getInstance();
}
#endif

std::weak_ptr<FileSystem> FileSystem::getInstance()
{
    std::scoped_lock<std::mutex> lock(m_singletonMutex);
    if (m_instance == nullptr)
    {
        m_instance.reset(new FileSystem);
    }
    return m_instance;
}

FileSystemIfReturnCode FileSystem::createPath(const std::filesystem::path &path, std::filesystem::perms mode, size_t pos, PathType type)
{
    std::error_code err;
    err.clear();
    try
    {
        if(exists(path))
        {
            return MAKE_RETURN_CODE(FileSystemIfReturnCode, ITC_FILESYSTEM_OK);
        }
        
        std::filesystem::path dirPath;
        if(type == PathType::DIRECTORY)
        {
            dirPath = path;
        } else if(type == PathType::REGULAR_FILE)
        {
            dirPath = path.parent_path();
        } else
        {
            return MAKE_RETURN_CODE(FileSystemIfReturnCode, ITC_FILESYSTEM_INVALID_ARGUMENTS);
        }
        
        std::filesystem::create_directories(dirPath, err);
        if(err)
        {
            std::cout << "Failed to create dirPath: " << dirPath << ", error: " << err.message() << std::endl;
            return MAKE_RETURN_CODE(FileSystemIfReturnCode, ITC_FILESYSTEM_SYSCALL_ERROR);
        }
        
        if(std::filesystem::is_regular_file(path))
        {
            std::ofstream file(path);
            if (!file)
            {
                TPT_TRACE(TRACE_ERROR, SSTR("Failed to create filePath = ", path));
                return MAKE_RETURN_CODE(FileSystemIfReturnCode, ITC_FILESYSTEM_SYSCALL_ERROR);
            }
        }
        
        std::filesystem::path subPath;
        size_t i = 0;
        for(const auto &component : path)
        {
            subPath /= component;
            if(i >= pos)
            {
                err.clear();
                std::filesystem::permissions(subPath, mode, err);
                if(err)
                {
                    std::cout << "Failed to grant permission path: " << subPath << ", error: " << err.message() << std::endl;
                    return MAKE_RETURN_CODE(FileSystemIfReturnCode, ITC_FILESYSTEM_SYSCALL_ERROR); 
                }
            }
            ++i;
        }
    } catch(const std::filesystem::filesystem_error& e)
    {
        std::cout << "Exception thrown: " << e.what() << std::endl;
        return MAKE_RETURN_CODE(FileSystemIfReturnCode, ITC_FILESYSTEM_EXCEPTION_THROWN);
    }
    
    return MAKE_RETURN_CODE(FileSystemIfReturnCode, ITC_FILESYSTEM_OK);
}

FileSystemIfReturnCode FileSystem::removePath(const std::filesystem::path &path)
{
    try
    {
        if(std::filesystem::exists(path))
        {
            if(std::filesystem::is_directory(path))
            {
                std::filesystem::remove_all(path);
            } else if(std::filesystem::is_regular_file(path))
            {
                std::filesystem::remove(path);
            }
        }
    } catch(const std::filesystem::filesystem_error& e)
    {
        std::cout << "removeDirectory: Exception: " << e.what() << std::endl;
        return MAKE_RETURN_CODE(FileSystemIfReturnCode, ITC_FILESYSTEM_EXCEPTION_THROWN);
    }
    return MAKE_RETURN_CODE(FileSystemIfReturnCode, ITC_FILESYSTEM_OK);
}

bool FileSystem::exists(const std::filesystem::path &path)
{
    try
    {
        if(std::filesystem::exists(path))
        {
            return true;
        }
    } catch(const std::filesystem::filesystem_error& e)
    {
        std::cout << "exists: Exception: " << e.what() << std::endl;
    }
    return false;
}

bool FileSystem::isAccessible(const std::filesystem::path &path)
{
    auto cWrapperIf = CWrapperIf::getInstance().lock();
    if(cWrapperIf && !cWrapperIf->cAccess(path.string().c_str(), F_OK | R_OK | W_OK))
    {
        return true;
    }
    return false;
}


} // namespace INTERNAL
} // namespace ItcPlatform
