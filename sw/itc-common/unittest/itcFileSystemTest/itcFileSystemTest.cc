#include "itcFileSystem.h"

#include <iostream>
#include <memory>
#include <string>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

#include "itc.h"
#include "itcConstant.h"
#include "itcCWrapperIfMock.h"

namespace ITC
{
namespace INTERNAL
{
using namespace ::testing;
using ::testing::AtLeast;
using PathType = FileSystemIf::PathType;
using namespace ITC::PROVIDED;
using FileSystemIfReturnCode = FileSystemIf::FileSystemIfReturnCode;

class FileSystemIfTest : public testing::Test
{
protected:
    FileSystemIfTest()
    {}

    ~FileSystemIfTest()
    {}
    
    void SetUp() override
    {
        m_fileSystem = FileSystem::getInstance().lock();
        m_cWrapperIfMock = CWrapperIfMock::getInstance().lock();
    }

    void TearDown() override
    {
        // m_fileSystem->m_instance.reset();
        // m_cWrapperIfMock->m_instance.reset();
    }
    
    bool verifyPath(const std::filesystem::path &path, std::filesystem::perms mode)
    {
        bool isPathCreatedWithCorrectMode {false};
        if(m_fileSystem->exists(path))
        {
            if(mode == std::filesystem::perms::unknown)
            {
                /* No need to check permission, instead check existence only. */
                return true;
            }
            
            struct stat info;
            if (stat(path.string().c_str(), &info) == 0)
            {
                isPathCreatedWithCorrectMode = (info.st_mode & 0777)  == \
                            static_cast<std::underlying_type_t<std::filesystem::perms>>(mode);
            }
        }
        return isPathCreatedWithCorrectMode;
    }
    
    bool createAndVerifyPath(const std::filesystem::path &path, PathType type, size_t pos = 0, std::filesystem::perms mode = std::filesystem::perms::all)
    {
        m_fileSystem->createPath(path, type, pos, mode);
        return verifyPath(path, mode);
    }
protected:
    std::shared_ptr<FileSystem> m_fileSystem;
    std::shared_ptr<CWrapperIfMock> m_cWrapperIfMock;
};

TEST_F(FileSystemIfTest, removePathTest1)
{
    /***
     * Test scenario: test remove a file path successfully.
     */
    std::filesystem::path filePath {"/tmp/abc.txt"};
    std::ofstream file(filePath);
    auto rc = m_fileSystem->removePath(filePath);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(FileSystemIfReturnCode, ITC_FILESYSTEM_OK));
    ASSERT_EQ(verifyPath(filePath, std::filesystem::perms::all), false);
}

TEST_F(FileSystemIfTest, removePathTest2)
{
    /***
     * Test scenario: test remove a not-permitted path.
     */
    std::filesystem::path path {"/etc"};
    auto rc = m_fileSystem->removePath(path);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(FileSystemIfReturnCode, ITC_FILESYSTEM_EXCEPTION_THROWN));
    ASSERT_EQ(verifyPath(path, std::filesystem::perms::unknown), true);
}

TEST_F(FileSystemIfTest, createPathTest1)
{
    /***
     * Test scenario: test create recursive directories without ending with a slash.
     */
    auto isPathCreatedWithCorrectMode = createAndVerifyPath("/tmp/abc/def/ghi", PathType::DIRECTORY, ITC_PATH_ITC_DIRECTORY_POSITION);
    ASSERT_EQ(isPathCreatedWithCorrectMode, true);
    m_fileSystem->removePath("/tmp/abc");
}

TEST_F(FileSystemIfTest, createPathTest2)
{
    /***
     * Test scenario: test create recursive directories with ending with a slash.
     */
    auto isPathCreatedWithCorrectMode = createAndVerifyPath("/tmp/abc/def/ghi/", PathType::DIRECTORY, ITC_PATH_ITC_DIRECTORY_POSITION);
    ASSERT_EQ(isPathCreatedWithCorrectMode, true);
    m_fileSystem->removePath("/tmp/abc");
}

TEST_F(FileSystemIfTest, createPathTest3)
{
    /***
     * Test scenario: test create an already exist directory.
     */
    constexpr size_t PATH_TMP_DIRECTORY_POSITION = 1;
    ASSERT_EQ(createAndVerifyPath("/tmp", PathType::DIRECTORY, PATH_TMP_DIRECTORY_POSITION), true);
}

TEST_F(FileSystemIfTest, createPathTest4)
{
    /***
     * Test scenario: test create an not-permitted directory.
     */
    constexpr size_t PATH_ETC_DIRECTORY_POSITION = 1;
    ASSERT_EQ(createAndVerifyPath("/etc", PathType::DIRECTORY, PATH_ETC_DIRECTORY_POSITION), false);
}

TEST_F(FileSystemIfTest, existsTest1)
{
    /***
     * Test scenario: test check an existing file.
     */
    ASSERT_EQ(m_fileSystem->exists("/etc"), true);
}

TEST_F(FileSystemIfTest, existsTest2)
{
    /***
     * Test scenario: test check a non-existing file.
     */
    ASSERT_EQ(m_fileSystem->exists("/ecc"), false);
}

TEST_F(FileSystemIfTest, isAccessibleTest1)
{
    /***
     * Test scenario: test can access an accessible file.
     */
    ASSERT_EQ(m_fileSystem->isAccessible("/tmp"), true);
}

TEST_F(FileSystemIfTest, isAccessibleTest2)
{
    /***
     * Test scenario: test can access a non-accessible file.
     */
    ASSERT_EQ(m_fileSystem->isAccessible("/etc"), false);
}

} // namespace INTERNAL
} // namespace ITC