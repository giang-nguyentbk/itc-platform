#include "itcTransportLSocket.h"

#include <iostream>
#include <memory>
#include <string>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "itcFileSystemIfMock.h"
#include "itcCWrapperIfMock.h"
#include "itcEthernetProto.h"

namespace ItcPlatform
{
namespace INTERNAL
{

using ::testing::AtLeast;
using ::testing::Pointee;
using namespace testing;
using namespace ItcPlatform::PROVIDED;
using ItcTransportIfReturnCode = ItcTransportIf::ItcTransportIfReturnCode;

class ItcTransportLSocketTest : public testing::Test
{
protected:
    ItcTransportLSocketTest()
    {}

    ~ItcTransportLSocketTest()
    {}
    
    void SetUp() override
    {
        m_transportLSocket = ItcTransportLSocket::getInstance().lock();
        m_fileSystemIfMock = FileSystemIfMock::getInstance().lock();
        m_cWrapperIfMock = CWrapperIfMock::getInstance().lock();
    }

    void TearDown() override
    {
        m_transportLSocket->m_instance.reset();
        m_fileSystemIfMock->m_instance.reset();
        m_cWrapperIfMock->m_instance.reset();
    }

protected:
    std::shared_ptr<ItcTransportLSocket> m_transportLSocket;
    std::shared_ptr<FileSystemIfMock> m_fileSystemIfMock;
    std::shared_ptr<CWrapperIfMock> m_cWrapperIfMock;
};

TEST_F(ItcTransportLSocketTest, initialiseTest1)
{
    /***
     * Test scenario: test itc-server was not running.
     */
    m_transportLSocket->m_isItcServerRunning = false;
    auto rc = m_transportLSocket->initialise();
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_ITC_SERVER_NOT_RUNNING));
}

TEST_F(ItcTransportLSocketTest, initialiseTest2)
{
    /***
     * Test scenario: test itc-server already running but LSocket path was not created yet,
     * then create the path, but failed.
     */
    m_transportLSocket->m_isItcServerRunning = true;
    m_transportLSocket->m_isLSockPathCreated = false;
    EXPECT_CALL(*m_fileSystemIfMock, createPath(_, _, _, _)).Times(1).WillOnce(Return(MAKE_RETURN_CODE(FileSystemIfReturnCode, ITC_FILESYSTEM_SYSCALL_ERROR)));
    auto rc = m_transportLSocket->initialise();
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_SYSCALL_ERROR));
    ASSERT_EQ(m_transportLSocket->m_isLSockPathCreated, false);
}

TEST_F(ItcTransportLSocketTest, initialiseTest3)
{
    /***
     * Test scenario: test cSocket() failed.
     */
    m_transportLSocket->m_isItcServerRunning = true;
    m_transportLSocket->m_isLSockPathCreated = false;
    EXPECT_CALL(*m_fileSystemIfMock, createPath(_, _, _, _)).Times(1).WillOnce(Return(MAKE_RETURN_CODE(FileSystemIfReturnCode, ITC_FILESYSTEM_OK)));
    EXPECT_CALL(*m_cWrapperIfMock, cSocket(_, _, _)).Times(1).WillOnce(Return(-1));
    auto rc = m_transportLSocket->initialise();
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_SYSCALL_ERROR));
    ASSERT_EQ(m_transportLSocket->m_isLSockPathCreated, true);
}

TEST_F(ItcTransportLSocketTest, initialiseTest4)
{
    /***
     * Test scenario: test cConnect() failed.
     */
    m_transportLSocket->m_isItcServerRunning = true;
    m_transportLSocket->m_isLSockPathCreated = false;
    itc_mailbox_id_t regionId = ITC_NO_MAILBOX_ID;
    int32_t mockedSockFd = 4;
    std::stringstream itcServerAddr;
    itcServerAddr << ITC_PATH_LSOCK_BASE_FILE_NAME << "_0x" << std::setw(8) << std::setfill('0') << std::hex << regionId;
    
    EXPECT_CALL(*m_fileSystemIfMock, createPath(_, _, _, _)).Times(1).WillOnce(Return(MAKE_RETURN_CODE(FileSystemIfReturnCode, ITC_FILESYSTEM_OK)));
    EXPECT_CALL(*m_cWrapperIfMock, cSocket(_, _, _)).Times(1).WillOnce(Return(mockedSockFd));
    EXPECT_CALL(*m_cWrapperIfMock, cConnect(mockedSockFd,
    Truly([&itcServerAddr](const sockaddr* addr)
    {
        auto *un = reinterpret_cast<const sockaddr_un*>(addr);
        return un->sun_family == AF_LOCAL && std::string(un->sun_path) == itcServerAddr.str();
    }), sizeof(sockaddr_un))).Times(1).WillOnce(Return(-1));
    
    auto rc = m_transportLSocket->initialise(regionId);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_SYSCALL_ERROR));
    ASSERT_EQ(m_transportLSocket->m_isLSockPathCreated, true);
}

TEST_F(ItcTransportLSocketTest, initialiseTest5)
{
    /***
     * Test scenario: test rxLen < 0.
     */
    m_transportLSocket->m_isItcServerRunning = true;
    m_transportLSocket->m_isLSockPathCreated = false;
    itc_mailbox_id_t regionId = ITC_NO_MAILBOX_ID;
    int32_t mockedSockFd = 4;
    std::stringstream itcServerAddr;
    itcServerAddr << ITC_PATH_LSOCK_BASE_FILE_NAME << "_0x" << std::setw(8) << std::setfill('0') << std::hex << regionId;
    
    EXPECT_CALL(*m_fileSystemIfMock, createPath(_, _, _, _)).Times(1).WillOnce(Return(MAKE_RETURN_CODE(FileSystemIfReturnCode, ITC_FILESYSTEM_OK)));
    EXPECT_CALL(*m_cWrapperIfMock, cSocket(_, _, _)).Times(1).WillOnce(Return(mockedSockFd));
    EXPECT_CALL(*m_cWrapperIfMock, cConnect(mockedSockFd,
    Truly([&itcServerAddr](const sockaddr* addr)
    {
        auto *un = reinterpret_cast<const sockaddr_un*>(addr);
        return un->sun_family == AF_LOCAL && std::string(un->sun_path) == itcServerAddr.str();
    }), sizeof(sockaddr_un))).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*m_cWrapperIfMock, cRecv(mockedSockFd, _, 4, 0)).Times(1).WillOnce(Return(-1));
    
    auto rc = m_transportLSocket->initialise(regionId);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_SYSCALL_ERROR));
    ASSERT_EQ(m_transportLSocket->m_isLSockPathCreated, true);
}

TEST_F(ItcTransportLSocketTest, initialiseTest6)
{
    /***
     * Test scenario: test rxLen >= 0 but != 4.
     */
    m_transportLSocket->m_isItcServerRunning = true;
    m_transportLSocket->m_isLSockPathCreated = false;
    itc_mailbox_id_t regionId = ITC_NO_MAILBOX_ID;
    int32_t mockedSockFd = 4;
    std::stringstream itcServerAddr;
    itcServerAddr << ITC_PATH_LSOCK_BASE_FILE_NAME << "_0x" << std::setw(8) << std::setfill('0') << std::hex << regionId;
    
    EXPECT_CALL(*m_fileSystemIfMock, createPath(_, _, _, _)).Times(1).WillOnce(Return(MAKE_RETURN_CODE(FileSystemIfReturnCode, ITC_FILESYSTEM_OK)));
    EXPECT_CALL(*m_cWrapperIfMock, cSocket(_, _, _)).Times(1).WillOnce(Return(mockedSockFd));
    EXPECT_CALL(*m_cWrapperIfMock, cConnect(mockedSockFd,
    Truly([&itcServerAddr](const sockaddr* addr)
    {
        auto *un = reinterpret_cast<const sockaddr_un*>(addr);
        return un->sun_family == AF_LOCAL && std::string(un->sun_path) == itcServerAddr.str();
    }), sizeof(sockaddr_un))).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*m_cWrapperIfMock, cRecv(mockedSockFd, _, 4, 0)).Times(1).WillOnce(Return(3));
    
    auto rc = m_transportLSocket->initialise(regionId);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_UNEXPECTED_RESPONSE));
    ASSERT_EQ(m_transportLSocket->m_isLSockPathCreated, true);
}

TEST_F(ItcTransportLSocketTest, initialiseTest7)
{
    /***
     * Test scenario: test rxLen == 4 but ack[4] != "ack".
     */
    m_transportLSocket->m_isItcServerRunning = true;
    m_transportLSocket->m_isLSockPathCreated = false;
    itc_mailbox_id_t regionId = ITC_NO_MAILBOX_ID;
    int32_t mockedSockFd = 4;
    std::stringstream itcServerAddr;
    itcServerAddr << ITC_PATH_LSOCK_BASE_FILE_NAME << "_0x" << std::setw(8) << std::setfill('0') << std::hex << regionId;
    char mockedAck[4] = "kca";
    
    EXPECT_CALL(*m_fileSystemIfMock, createPath(_, _, _, _)).Times(1).WillOnce(Return(MAKE_RETURN_CODE(FileSystemIfReturnCode, ITC_FILESYSTEM_OK)));
    EXPECT_CALL(*m_cWrapperIfMock, cSocket(_, _, _)).Times(1).WillOnce(Return(mockedSockFd));
    EXPECT_CALL(*m_cWrapperIfMock, cConnect(mockedSockFd,
    Truly([&itcServerAddr](const sockaddr* addr)
    {
        auto *un = reinterpret_cast<const sockaddr_un*>(addr);
        return un->sun_family == AF_LOCAL && std::string(un->sun_path) == itcServerAddr.str();
    }), sizeof(sockaddr_un))).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*m_cWrapperIfMock, cRecv(mockedSockFd, _, 4, 0)).Times(1).WillOnce(DoAll(
    /***
     * Cannot use SetArrayArgument here because the 2nd argument is "void *",
     * not a specific pointer-to-object.
     */
    // SetArrayArgument<1>(mockedAck, mockedAck + 4),
    // Return(4)
    WithArg<1>([&mockedAck](void* buf)
    { 
        auto *charBuf = reinterpret_cast<char *>(buf);
        ::strcpy(charBuf, mockedAck);
    }),Return(4)));
    
    auto rc = m_transportLSocket->initialise(regionId);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_UNEXPECTED_RESPONSE));
    ASSERT_EQ(m_transportLSocket->m_isLSockPathCreated, true);
}

TEST_F(ItcTransportLSocketTest, initialiseTest8)
{
    /***
     * Test scenario: test happy case.
     */
    m_transportLSocket->m_isItcServerRunning = true;
    m_transportLSocket->m_isLSockPathCreated = false;
    itc_mailbox_id_t regionId = ITC_NO_MAILBOX_ID;
    int32_t mockedSockFd = 4;
    std::stringstream itcServerAddr;
    itcServerAddr << ITC_PATH_LSOCK_BASE_FILE_NAME << "_0x" << std::setw(8) << std::setfill('0') << std::hex << regionId;
    char mockedAck[4] = "ack";
    
    EXPECT_CALL(*m_fileSystemIfMock, createPath(_, _, _, _)).Times(1).WillOnce(Return(MAKE_RETURN_CODE(FileSystemIfReturnCode, ITC_FILESYSTEM_OK)));
    EXPECT_CALL(*m_cWrapperIfMock, cSocket(_, _, _)).Times(1).WillOnce(Return(mockedSockFd));
    EXPECT_CALL(*m_cWrapperIfMock, cConnect(mockedSockFd,
    Truly([&itcServerAddr](const sockaddr* addr)
    {
        auto *un = reinterpret_cast<const sockaddr_un*>(addr);
        return un->sun_family == AF_LOCAL && std::string(un->sun_path) == itcServerAddr.str();
    }), sizeof(sockaddr_un))).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*m_cWrapperIfMock, cRecv(mockedSockFd, _, 4, 0)).Times(1).WillOnce(DoAll(
    /***
     * Cannot use SetArrayArgument here because the 2nd argument is "void *",
     * not a specific pointer-to-object.
     */
    // SetArrayArgument<1>(mockedAck, mockedAck + 4),
    // Return(4)
    WithArg<1>([&mockedAck](void* buf)
    { 
        auto *charBuf = reinterpret_cast<char *>(buf);
        ::strcpy(charBuf, mockedAck);
    }),Return(4)));
    
    auto rc = m_transportLSocket->initialise(regionId);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_OK));
    ASSERT_EQ(m_transportLSocket->m_isLSockPathCreated, true);
    ASSERT_EQ(m_transportLSocket->m_sockFd, mockedSockFd);
}

TEST_F(ItcTransportLSocketTest, releaseTest1)
{
    /***
     * Test scenario: test itc-server was not running.
     */
    m_transportLSocket->m_isItcServerRunning = false;
    auto rc = m_transportLSocket->release();
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_OK));
}

TEST_F(ItcTransportLSocketTest, releaseTest2)
{
    /***
     * Test scenario: test m_sockFd is invalid, then cClose() return -1 as well.
     */
    m_transportLSocket->m_isItcServerRunning = true;
    m_transportLSocket->m_sockFd = -1;
    EXPECT_CALL(*m_cWrapperIfMock, cClose(-1)).Times(1).WillOnce(Return(-1));
    
    auto rc = m_transportLSocket->release();
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_SYSCALL_ERROR));
}

TEST_F(ItcTransportLSocketTest, releaseTest3)
{
    /***
     * Test scenario: test cannot remove socket folder.
     */
    m_transportLSocket->m_isItcServerRunning = true;
    m_transportLSocket->m_sockFd = 4;
    const std::string socketFolderPath = ITC_PATH_SOCK_FOLDER_NAME;
    EXPECT_CALL(*m_cWrapperIfMock, cClose(4)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*m_fileSystemIfMock, removePath(
    Truly(
        [&socketFolderPath](const std::filesystem::path &path)
        {
            return path.string() == socketFolderPath;
        }
    ))).Times(1).WillOnce(Return(MAKE_RETURN_CODE(FileSystemIfReturnCode, ITC_FILESYSTEM_EXCEPTION_THROWN)));
    
    auto rc = m_transportLSocket->release();
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_SYSCALL_ERROR));
    // ASSERT_EQ(m_transportLSocket->m_sockFd, -1);
    // ASSERT_EQ(m_transportLSocket->m_isItcServerRunning, false);
    // ASSERT_EQ(m_transportLSocket->m_isLSockPathCreated, false);
}

TEST_F(ItcTransportLSocketTest, releaseTest4)
{
    /***
     * Test scenario: test happy case.
     */
    m_transportLSocket->m_isItcServerRunning = true;
    m_transportLSocket->m_sockFd = 4;
    const std::string socketFolderPath = ITC_PATH_SOCK_FOLDER_NAME;
    EXPECT_CALL(*m_cWrapperIfMock, cClose(4)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*m_fileSystemIfMock, removePath(
    Truly(
        [&socketFolderPath](const std::filesystem::path &path)
        {
            return path.string() == socketFolderPath;
        }
    ))).Times(1).WillOnce(Return(MAKE_RETURN_CODE(FileSystemIfReturnCode, ITC_FILESYSTEM_OK)));
    
    auto rc = m_transportLSocket->release();
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_OK));
    ASSERT_EQ(m_transportLSocket->m_sockFd, -1);
    ASSERT_EQ(m_transportLSocket->m_isItcServerRunning, false);
    ASSERT_EQ(m_transportLSocket->m_isLSockPathCreated, false);
}

TEST_F(ItcTransportLSocketTest, locateItcServerTest1)
{
    /***
     * Test scenario: test failed to cSocket().
     */
    itc_mailbox_id_t assignedRegionId = ITC_NO_MAILBOX_ID;
    itc_mailbox_id_t locatedItcServerMboxId = ITC_NO_MAILBOX_ID;
    
    EXPECT_CALL(*m_cWrapperIfMock, cSocket(_, _, _)).Times(1).WillOnce(Return(-1));
    
    auto rc = m_transportLSocket->locateItcServer(&assignedRegionId, &locatedItcServerMboxId);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_SYSCALL_ERROR));
}

TEST_F(ItcTransportLSocketTest, locateItcServerTest2)
{
    /***
     * Test scenario: test failed to cConnect().
     */
    itc_mailbox_id_t assignedRegionId = ITC_NO_MAILBOX_ID;
    itc_mailbox_id_t locatedItcServerMboxId = ITC_NO_MAILBOX_ID;
    int32_t mockedSockFd = 4;
    const std::string itcServerFileName = ITC_PATH_ITC_SERVER_FILE_NAME;
    
    EXPECT_CALL(*m_cWrapperIfMock, cSocket(_, _, _)).Times(1).WillOnce(Return(mockedSockFd));
    EXPECT_CALL(*m_cWrapperIfMock, cConnect(mockedSockFd,
    Truly([&itcServerFileName](const sockaddr* addr)
    {
        auto *un = reinterpret_cast<const sockaddr_un*>(addr);
        return un->sun_family == AF_LOCAL && std::string(un->sun_path) == itcServerFileName;
    }), sizeof(sockaddr_un))).Times(1).WillOnce(Return(-1));
    
    auto rc = m_transportLSocket->locateItcServer(&assignedRegionId, &locatedItcServerMboxId);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_SYSCALL_ERROR));
}

TEST_F(ItcTransportLSocketTest, locateItcServerTest3)
{
    /***
     * Test scenario: test failed to cSend().
     */
    itc_mailbox_id_t assignedRegionId = ITC_NO_MAILBOX_ID;
    itc_mailbox_id_t locatedItcServerMboxId = ITC_NO_MAILBOX_ID;
    int32_t mockedSockFd = 4;
    const std::string itcServerFileName = ITC_PATH_ITC_SERVER_FILE_NAME;
    
    EXPECT_CALL(*m_cWrapperIfMock, cSocket(_, _, _)).Times(1).WillOnce(Return(mockedSockFd));
    EXPECT_CALL(*m_cWrapperIfMock, cConnect(mockedSockFd,
    Truly([&itcServerFileName](const sockaddr* addr)
    {
        auto *un = reinterpret_cast<const sockaddr_un*>(addr);
        return un->sun_family == AF_LOCAL && std::string(un->sun_path) == itcServerFileName;
    }), sizeof(sockaddr_un))).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*m_cWrapperIfMock, cSend(mockedSockFd, _, _, _)).Times(1).WillOnce(Return(-1));
    
    auto rc = m_transportLSocket->locateItcServer(&assignedRegionId, &locatedItcServerMboxId);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_SYSCALL_ERROR));
}

TEST_F(ItcTransportLSocketTest, locateItcServerTest4)
{
    /***
     * Test scenario: test receivedBytes < 0.
     */
    itc_mailbox_id_t assignedRegionId = ITC_NO_MAILBOX_ID;
    itc_mailbox_id_t locatedItcServerMboxId = ITC_NO_MAILBOX_ID;
    int32_t mockedSockFd = 4;
    const std::string itcServerFileName = ITC_PATH_ITC_SERVER_FILE_NAME;
    
    EXPECT_CALL(*m_cWrapperIfMock, cSocket(_, _, _)).Times(1).WillOnce(Return(mockedSockFd));
    EXPECT_CALL(*m_cWrapperIfMock, cConnect(mockedSockFd,
    Truly([&itcServerFileName](const sockaddr* addr)
    {
        auto *un = reinterpret_cast<const sockaddr_un*>(addr);
        return un->sun_family == AF_LOCAL && std::string(un->sun_path) == itcServerFileName;
    }), sizeof(sockaddr_un))).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*m_cWrapperIfMock, cSend(mockedSockFd, _, _, _)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*m_cWrapperIfMock, cRecv(mockedSockFd, _, _, _)).Times(1).WillOnce(Return(-1));
    
    auto rc = m_transportLSocket->locateItcServer(&assignedRegionId, &locatedItcServerMboxId);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_INVALID_RESPONSE));
}

TEST_F(ItcTransportLSocketTest, locateItcServerTest5)
{
    /***
     * Test scenario: test happy case.
     */
    itc_mailbox_id_t assignedRegionId = ITC_NO_MAILBOX_ID;
    itc_mailbox_id_t locatedItcServerMboxId = ITC_NO_MAILBOX_ID;
    int32_t mockedSockFd = 4;
    itc_mailbox_id_t mockedRegionId = 0x00300000;
    itc_mailbox_id_t mockedItcServerMboxId = 0x00100001;
    const std::string itcServerFileName = ITC_PATH_ITC_SERVER_FILE_NAME;
    
    EXPECT_CALL(*m_cWrapperIfMock, cSocket(_, _, _)).Times(1).WillOnce(Return(mockedSockFd));
    EXPECT_CALL(*m_cWrapperIfMock, cConnect(mockedSockFd,
    Truly([&itcServerFileName](const sockaddr* addr)
    {
        auto *un = reinterpret_cast<const sockaddr_un*>(addr);
        return un->sun_family == AF_LOCAL && std::string(un->sun_path) == itcServerFileName;
    }), sizeof(sockaddr_un))).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*m_cWrapperIfMock, cSend(mockedSockFd, _, _, _)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*m_cWrapperIfMock, cRecv(mockedSockFd, _, _, 0)).Times(1).WillOnce(DoAll(
    WithArg<1>([&mockedRegionId, &mockedItcServerMboxId](void* buf)
    { 
        auto *lreply = reinterpret_cast<itc_ethernet_message_locate_itc_server_reply *>(buf);
        lreply->msgno = ITC_ETHERNET_MESSAGE_LOCATE_ITC_SERVER_REPLY;
        lreply->assignedRegionId = mockedRegionId;
        lreply->itcServerMboxId = mockedItcServerMboxId;
    }),Return((ssize_t)sizeof(itc_ethernet_message_locate_itc_server_reply))));
    EXPECT_CALL(*m_cWrapperIfMock, cClose(mockedSockFd)).Times(1).WillOnce(Return(0));
    
    auto rc = m_transportLSocket->locateItcServer(&assignedRegionId, &locatedItcServerMboxId);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_OK));
    ASSERT_EQ(m_transportLSocket->m_isItcServerRunning, true);
    ASSERT_EQ(assignedRegionId, mockedRegionId);
    ASSERT_EQ(locatedItcServerMboxId, mockedItcServerMboxId);
}

TEST_F(ItcTransportLSocketTest, locateItcServerTest6)
{
    /***
     * Test scenario: test malformed lsocket reply from itc-server.
     */
    itc_mailbox_id_t assignedRegionId = ITC_NO_MAILBOX_ID;
    itc_mailbox_id_t locatedItcServerMboxId = ITC_NO_MAILBOX_ID;
    int32_t mockedSockFd = 4;
    const std::string itcServerFileName = ITC_PATH_ITC_SERVER_FILE_NAME;
    
    EXPECT_CALL(*m_cWrapperIfMock, cSocket(_, _, _)).Times(1).WillOnce(Return(mockedSockFd));
    EXPECT_CALL(*m_cWrapperIfMock, cConnect(mockedSockFd,
    Truly([&itcServerFileName](const sockaddr* addr)
    {
        auto *un = reinterpret_cast<const sockaddr_un*>(addr);
        return un->sun_family == AF_LOCAL && std::string(un->sun_path) == itcServerFileName;
    }), sizeof(sockaddr_un))).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*m_cWrapperIfMock, cSend(mockedSockFd, _, _, _)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*m_cWrapperIfMock, cRecv(mockedSockFd, _, _, 0)).Times(1).WillOnce(DoAll(
    WithArg<1>([](void* buf)
    { 
        auto *lreply = reinterpret_cast<itc_ethernet_message_locate_itc_server_reply *>(buf);
        lreply->msgno = ITC_MESSAGE_NUMBER_UNDEFINED;
        lreply->assignedRegionId = ITC_NO_MAILBOX_ID;
    }),Return((ssize_t)sizeof(itc_ethernet_message_locate_itc_server_reply))));
    EXPECT_CALL(*m_cWrapperIfMock, cClose(mockedSockFd)).Times(1).WillOnce(Return(0));
    
    auto rc = m_transportLSocket->locateItcServer(&assignedRegionId, &locatedItcServerMboxId);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_INVALID_RESPONSE));
}


} // namespace INTERNAL
} // namespace ItcPlatform