#include "itcTransportSysvMsgQueue.h"

#include <iostream>
#include <memory>
#include <string>
#include <cerrno>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "itcFileSystemIfMock.h"
#include "itcCWrapperIfMock.h"
#include "itcPlatformIfMock.h"
#include "itcThreadManagerIfMock.h"

#include "itcEthernetProto.h"

namespace ITC
{
namespace INTERNAL
{

using ::testing::AtLeast;
using ::testing::Pointee;
using ::testing::InSequence;
using namespace testing;
using namespace ITC::PROVIDED;
using ItcTransportIfReturnCode = ItcTransportIf::ItcTransportIfReturnCode;
using ItcPlatformIfReturnCode = ItcPlatformIf::ItcPlatformIfReturnCode;

class ItcTransportSysvMsgQueueTest : public testing::Test
{
protected:
    ItcTransportSysvMsgQueueTest()
    {}

    ~ItcTransportSysvMsgQueueTest()
    {}
    
    void SetUp() override
    {
        m_transportSysvMsgQueue = ItcTransportSysvMsgQueue::getInstance().lock();
        m_fileSystemIfMock = FileSystemIfMock::getInstance().lock();
        m_cWrapperIfMock = CWrapperIfMock::getInstance().lock();
        m_itcPlatformIfMock = ItcPlatformIfMock::getInstance().lock();
        m_threadManagerIfMock = ThreadManagerIfMock::getInstance().lock();
        errno = 0;
    }

    void TearDown() override
    {
        m_transportSysvMsgQueue->m_instance.reset();
        m_fileSystemIfMock->m_instance.reset();
        m_cWrapperIfMock->m_instance.reset();
        m_itcPlatformIfMock->m_instance.reset();
        m_threadManagerIfMock->m_instance.reset();
        errno = 0;
    }

protected:
    std::shared_ptr<ItcTransportSysvMsgQueue> m_transportSysvMsgQueue;
    std::shared_ptr<FileSystemIfMock> m_fileSystemIfMock;
    std::shared_ptr<CWrapperIfMock> m_cWrapperIfMock;
    std::shared_ptr<ItcPlatformIfMock> m_itcPlatformIfMock;
    std::shared_ptr<ThreadManagerIfMock> m_threadManagerIfMock;
};

TEST_F(ItcTransportSysvMsgQueueTest, releaseAllResourcesTest1)
{
    /***
     * Test scenario: test failed to delete pthread key.
     */
    pthread_key_t mockedKey = 99;
    m_transportSysvMsgQueue->m_destructKey = mockedKey;
    EXPECT_CALL(*m_cWrapperIfMock, cPthreadKeyDelete(mockedKey)).Times(1).WillOnce(Return(-1));
    
    auto rc = m_transportSysvMsgQueue->releaseAllResources();
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_SYSCALL_ERROR));
}

TEST_F(ItcTransportSysvMsgQueueTest, releaseAllResourcesTest2)
{
    /***
     * Test scenario: test happy case.
     */
    pthread_key_t mockedKey = 99;
    m_transportSysvMsgQueue->m_destructKey = mockedKey;
    EXPECT_CALL(*m_cWrapperIfMock, cPthreadKeyDelete(mockedKey)).Times(1).WillOnce(Return(0));
    
    auto rc = m_transportSysvMsgQueue->releaseAllResources();
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_OK));
    ASSERT_EQ(m_transportSysvMsgQueue->m_isTerminated, true);
}

TEST_F(ItcTransportSysvMsgQueueTest, getMsgQueueIdTest1)
{
    /***
     * Test scenario: test failed to ftok.
     */
    itc_mailbox_id_t mboxId = 0x00100004;
    itc_mailbox_id_t newMboxId = mboxId & ITC_MASK_REGION_ID;
	itc_mailbox_id_t projectId = newMboxId >> ITC_REGION_ID_SHIFT;
    EXPECT_CALL(*m_cWrapperIfMock, cFtok(
    Truly(
        [](const char *pathname)
        {
            return std::string(pathname) == ITC_PATH_SYSVMQ_FILE_NAME;
        }
    ), projectId)).Times(1).WillOnce(Return(-1));
    
    auto msgQueueId = m_transportSysvMsgQueue->getMsgQueueId(mboxId);
    ASSERT_EQ(msgQueueId, -1);
}

TEST_F(ItcTransportSysvMsgQueueTest, getMsgQueueIdTest2)
{
    /***
     * Test scenario: test failed to msgget.
     */
    itc_mailbox_id_t mboxId = 0x00100004;
    itc_mailbox_id_t newMboxId = mboxId & ITC_MASK_REGION_ID;
	itc_mailbox_id_t projectId = newMboxId >> ITC_REGION_ID_SHIFT;
    key_t mockedKey = 4;
    EXPECT_CALL(*m_cWrapperIfMock, cFtok(
    Truly(
        [](const char *pathname)
        {
            return std::string(pathname) == ITC_PATH_SYSVMQ_FILE_NAME;
        }
    ), projectId)).Times(1).WillOnce(Return(mockedKey));
    EXPECT_CALL(*m_cWrapperIfMock, cMsgGet(mockedKey, 0)).Times(1).WillOnce(Return(-1));
    
    auto msgQueueId = m_transportSysvMsgQueue->getMsgQueueId(mboxId);
    ASSERT_EQ(msgQueueId, -1);
}

TEST_F(ItcTransportSysvMsgQueueTest, getMsgQueueIdTest3)
{
    /***
     * Test scenario: test happy case.
     */
    itc_mailbox_id_t mboxId = 0x00100004;
    itc_mailbox_id_t newMboxId = mboxId & ITC_MASK_REGION_ID;
	itc_mailbox_id_t projectId = newMboxId >> ITC_REGION_ID_SHIFT;
    key_t mockedKey = 4;
    int32_t mockedMsgQueueId = 10;
    EXPECT_CALL(*m_cWrapperIfMock, cFtok(
    Truly(
        [](const char *pathname)
        {
            return std::string(pathname) == ITC_PATH_SYSVMQ_FILE_NAME;
        }
    ), projectId)).Times(1).WillOnce(Return(mockedKey));
    EXPECT_CALL(*m_cWrapperIfMock, cMsgGet(mockedKey, 0)).Times(1).WillOnce(Return(mockedMsgQueueId));
    
    auto msgQueueId = m_transportSysvMsgQueue->getMsgQueueId(mboxId);
    ASSERT_EQ(msgQueueId, mockedMsgQueueId);
}

TEST_F(ItcTransportSysvMsgQueueTest, parseAndForwardMessageTest1)
{
    /***
     * Test scenario: test invalid sysv tx msgno.
     */
    itc_mailbox_id_t sender = 0x00100001;
    itc_mailbox_id_t receiver = 0x00100002;
    uint32_t msgno = 0x12345678;
    
    int32_t msgSize = ITC_ADMIN_MESSAGE_MIN_SIZE;
	uint8_t *txBuffer = new uint8_t[sizeof(long) + msgSize];
    auto txMsg {reinterpret_cast<long *>(txBuffer)};
	*txMsg = (long)ITC_SYSV_MESSAGE_QUEUE_TX_MSGNO + 1; /* Force wrong sysv tx msgno */
	auto adminMsg = reinterpret_cast<ItcAdminMessageRawPtr>(txBuffer + sizeof(long));
    adminMsg->sender = sender;
    adminMsg->receiver = receiver;
    adminMsg->msgno = msgno;
    adminMsg->size = sizeof(msgno);
    uint8_t *endpoint = reinterpret_cast<uint8_t *>(adminMsg) + msgSize - 1;
    *endpoint = ITC_ADMIN_MESSAGE_ENDPOINT;
    auto *rxMsg = txBuffer;
    
    auto ret = m_transportSysvMsgQueue->parseAndForwardMessage(rxMsg, sizeof(long) + msgSize);
    ASSERT_EQ(ret, -1);
    delete[] txBuffer;
}

TEST_F(ItcTransportSysvMsgQueueTest, parseAndForwardMessageTest2)
{
    /***
     * Test scenario: test rxMsg length too small.
     */
    itc_mailbox_id_t sender = 0x00100001;
    itc_mailbox_id_t receiver = 0x00100002;
    uint32_t msgno = 0x12345678;
    
    int32_t msgSize = ITC_ADMIN_MESSAGE_MIN_SIZE;
	uint8_t *txBuffer = new uint8_t[sizeof(long) + msgSize];
    auto txMsg {reinterpret_cast<long *>(txBuffer)};
	*txMsg = (long)ITC_SYSV_MESSAGE_QUEUE_TX_MSGNO;
	auto adminMsg = reinterpret_cast<ItcAdminMessageRawPtr>(txBuffer + sizeof(long));
    adminMsg->sender = sender;
    adminMsg->receiver = receiver;
    adminMsg->msgno = msgno;
    adminMsg->size = sizeof(msgno);
    uint8_t *endpoint = reinterpret_cast<uint8_t *>(adminMsg) + msgSize - 1;
    *endpoint = ITC_ADMIN_MESSAGE_ENDPOINT;
    auto *rxMsg = txBuffer;
    
    auto ret = m_transportSysvMsgQueue->parseAndForwardMessage(rxMsg, sizeof(long) + msgSize - 1); /* Force invalid rxLength */
    ASSERT_EQ(ret, -2);
    delete[] txBuffer;
}

TEST_F(ItcTransportSysvMsgQueueTest, parseAndForwardMessageTest3)
{
    /***
     * Test scenario: test invalid itc admin message's endpoint.
     */
    itc_mailbox_id_t sender = 0x00100001;
    itc_mailbox_id_t receiver = 0x00100002;
    uint32_t msgno = 0x12345678;
    
    int32_t msgSize = ITC_ADMIN_MESSAGE_MIN_SIZE;
	uint8_t *txBuffer = new uint8_t[sizeof(long) + msgSize];
    auto txMsg {reinterpret_cast<long *>(txBuffer)};
	*txMsg = (long)ITC_SYSV_MESSAGE_QUEUE_TX_MSGNO;
	auto adminMsg = reinterpret_cast<ItcAdminMessageRawPtr>(txBuffer + sizeof(long));
    adminMsg->sender = sender;
    adminMsg->receiver = receiver;
    adminMsg->msgno = msgno;
    adminMsg->size = sizeof(msgno);
    uint8_t *endpoint = reinterpret_cast<uint8_t *>(adminMsg) + msgSize - 1;
    *endpoint = ITC_ADMIN_MESSAGE_ENDPOINT - 1; /* Force invalid endpoint */
    auto *rxMsg = txBuffer;
    
    auto ret = m_transportSysvMsgQueue->parseAndForwardMessage(rxMsg, sizeof(long) + msgSize);
    ASSERT_EQ(ret, -3);
    delete[] txBuffer;
}

TEST_F(ItcTransportSysvMsgQueueTest, parseAndForwardMessageTest4)
{
    /***
     * Test scenario: test failed to forward message to local mailbox inside same process.
     */
    itc_mailbox_id_t sender = 0x00100001;
    itc_mailbox_id_t receiver = 0x00100002;
    uint32_t msgno = 0x12345678;
    uint32_t flags = 0xFFFF;
    
    int32_t msgSize = ITC_ADMIN_MESSAGE_MIN_SIZE;
	uint8_t *txBuffer = new uint8_t[sizeof(long) + msgSize];
    auto txMsg {reinterpret_cast<long *>(txBuffer)};
	*txMsg = (long)ITC_SYSV_MESSAGE_QUEUE_TX_MSGNO;
	auto adminMsg = reinterpret_cast<ItcAdminMessageRawPtr>(txBuffer + sizeof(long));
    adminMsg->sender = sender;
    adminMsg->receiver = receiver;
    adminMsg->msgno = msgno;
    adminMsg->size = sizeof(msgno);
    adminMsg->flags = ITC_FLAG_DEFAULT;
    uint8_t *endpoint = reinterpret_cast<uint8_t *>(adminMsg) + msgSize - 1;
    *endpoint = ITC_ADMIN_MESSAGE_ENDPOINT;
    auto *rxMsg = txBuffer;
    ItcAdminMessageRawPtr newMsg = reinterpret_cast<ItcAdminMessageRawPtr>(new uint8_t[msgSize]);
    newMsg->flags = flags;
    
    EXPECT_CALL(*m_itcPlatformIfMock, allocateMessage(msgno, ITC_MESSAGE_MSGNO_SIZE)).Times(1).WillOnce(Return(CONVERT_TO_USER_MESSAGE(newMsg)));
    EXPECT_CALL(*m_itcPlatformIfMock, send(CONVERT_TO_USER_MESSAGE(newMsg),
    Truly(
        [&receiver](const MailboxContactInfo &toMbox)
        {
            return toMbox.mailboxId == receiver;
        }
    ), ITC_MODE_MESSAGE_FORWARDING)).Times(1).WillOnce(Return(MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_FAILED)));
    
    auto ret = m_transportSysvMsgQueue->parseAndForwardMessage(rxMsg, sizeof(long) + msgSize);
    ASSERT_EQ(ret, -4);
    ASSERT_EQ(newMsg->flags, flags);
    ASSERT_EQ(newMsg->sender, adminMsg->sender);
    ASSERT_EQ(newMsg->receiver, adminMsg->receiver);
    ASSERT_EQ(newMsg->msgno, adminMsg->msgno);
    delete[] txBuffer;
    delete[] newMsg;
}

TEST_F(ItcTransportSysvMsgQueueTest, parseAndForwardMessageTest5)
{
    /***
     * Test scenario: test happy case.
     */
    itc_mailbox_id_t sender = 0x00100001;
    itc_mailbox_id_t receiver = 0x00100002;
    uint32_t msgno = 0x12345678;
    uint32_t flags = 0xFFFF;
    
    int32_t msgSize = ITC_ADMIN_MESSAGE_MIN_SIZE;
	uint8_t *txBuffer = new uint8_t[sizeof(long) + msgSize];
    auto txMsg {reinterpret_cast<long *>(txBuffer)};
	*txMsg = (long)ITC_SYSV_MESSAGE_QUEUE_TX_MSGNO;
	auto adminMsg = reinterpret_cast<ItcAdminMessageRawPtr>(txBuffer + sizeof(long));
    adminMsg->sender = sender;
    adminMsg->receiver = receiver;
    adminMsg->msgno = msgno;
    adminMsg->size = sizeof(msgno);
    adminMsg->flags = ITC_FLAG_DEFAULT;
    uint8_t *endpoint = reinterpret_cast<uint8_t *>(adminMsg) + msgSize - 1;
    *endpoint = ITC_ADMIN_MESSAGE_ENDPOINT;
    auto *rxMsg = txBuffer;
    ItcAdminMessageRawPtr newMsg = reinterpret_cast<ItcAdminMessageRawPtr>(new uint8_t[msgSize]);
    newMsg->flags = flags;
    
    EXPECT_CALL(*m_itcPlatformIfMock, allocateMessage(msgno, ITC_MESSAGE_MSGNO_SIZE)).Times(1).WillOnce(Return(CONVERT_TO_USER_MESSAGE(newMsg)));
    EXPECT_CALL(*m_itcPlatformIfMock, send(CONVERT_TO_USER_MESSAGE(newMsg),
    Truly(
        [&receiver](const MailboxContactInfo &toMbox)
        {
            return toMbox.mailboxId == receiver;
        }
    ), ITC_MODE_MESSAGE_FORWARDING)).Times(1).WillOnce(Return(MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_OK)));
    
    auto ret = m_transportSysvMsgQueue->parseAndForwardMessage(rxMsg, sizeof(long) + msgSize);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(newMsg->flags, flags);
    ASSERT_EQ(newMsg->sender, adminMsg->sender);
    ASSERT_EQ(newMsg->receiver, adminMsg->receiver);
    ASSERT_EQ(newMsg->msgno, adminMsg->msgno);
    delete[] txBuffer;
    delete[] newMsg;
}

TEST_F(ItcTransportSysvMsgQueueTest, getMaxMessageSizeTest1)
{
    /***
     * Test scenario: test already obtained maxMsgSize case.
     */
    m_transportSysvMsgQueue->m_maxMsgSize = 100;
    
    auto maxMsgSize = m_transportSysvMsgQueue->getMaxMessageSize();
    ASSERT_EQ(maxMsgSize, 100);
}

TEST_F(ItcTransportSysvMsgQueueTest, getMaxMessageSizeTest2)
{
    /***
     * Test scenario: test failed to msgctl.
     */
    m_transportSysvMsgQueue->m_maxMsgSize = 0;
    EXPECT_CALL(*m_cWrapperIfMock, cMsgctl(_, _, _)).Times(1).WillOnce(Return(-1));
    
    auto maxMsgSize = m_transportSysvMsgQueue->getMaxMessageSize();
    ASSERT_EQ(maxMsgSize, 0);
}

TEST_F(ItcTransportSysvMsgQueueTest, getMaxMessageSizeTest3)
{
    /***
     * Test scenario: test retrieve max msg size via info.msgmax.
     */
    int32_t mockedMsgSize = 10;
    m_transportSysvMsgQueue->m_maxMsgSize = 0;
    EXPECT_CALL(*m_cWrapperIfMock, cMsgctl(_, _, _)).Times(1).WillOnce(DoAll(
        WithArg<2>([&mockedMsgSize](struct msqid_ds *arg)
        {
            auto info = reinterpret_cast<struct msginfo *>(arg);
            info->msgmax = mockedMsgSize;
            info->msgmnb = mockedMsgSize + 10;
        }),
    Return(0)));
    
    auto maxMsgSize = m_transportSysvMsgQueue->getMaxMessageSize();
    ASSERT_EQ(maxMsgSize, mockedMsgSize);
}

TEST_F(ItcTransportSysvMsgQueueTest, initialiseTest1)
{
    /***
     * Test scenario: test initialse an already intialised sysv msgqueue transport without force re-init flag.
     */
    m_transportSysvMsgQueue->m_isInitialised = true;
    
    auto rc = m_transportSysvMsgQueue->initialise(ITC_MAILBOX_ID_DEFAULT, 1, ITC_FLAG_DEFAULT);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_ALREADY_INITIALISED));
    ASSERT_EQ(m_transportSysvMsgQueue->m_isInitialised, true);
}

TEST_F(ItcTransportSysvMsgQueueTest, initialiseTest2)
{
    /***
     * Test scenario: test force re-init transport, but createPath failed.
     */
    m_transportSysvMsgQueue->m_isInitialised = true;
    
    /* releaseAllResources happy case */
    pthread_key_t mockedKey = 99;
    m_transportSysvMsgQueue->m_destructKey = mockedKey;
    EXPECT_CALL(*m_cWrapperIfMock, cPthreadKeyDelete(mockedKey)).Times(1).WillOnce(Return(0));
    
    /* getMaxMessageSize already exist case */
    m_transportSysvMsgQueue->m_maxMsgSize = 1000;
    
    EXPECT_CALL(*m_fileSystemIfMock, createPath(_, _, _, _)).Times(1).WillOnce(Return(MAKE_RETURN_CODE(FileSystemIfReturnCode, ITC_FILESYSTEM_EXCEPTION_THROWN)));
    
    auto rc = m_transportSysvMsgQueue->initialise(ITC_MAILBOX_ID_DEFAULT, 1, ITC_FLAG_FORCE_REINIT_TRANSPORTS);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_SYSCALL_ERROR));
    ASSERT_EQ(m_transportSysvMsgQueue->m_isInitialised, false);
}

TEST_F(ItcTransportSysvMsgQueueTest, initialiseTest3)
{
    /***
     * Test scenario: test force re-init transport, but create pthread key failed.
     */
    m_transportSysvMsgQueue->m_isInitialised = true;
    
    /* releaseAllResources happy case */
    pthread_key_t mockedKey = 99;
    m_transportSysvMsgQueue->m_destructKey = mockedKey;
    EXPECT_CALL(*m_cWrapperIfMock, cPthreadKeyDelete(mockedKey)).Times(1).WillOnce(Return(0));
    
    /* getMaxMessageSize already exist case */
    m_transportSysvMsgQueue->m_maxMsgSize = 1000;
    
    EXPECT_CALL(*m_fileSystemIfMock, createPath(_, _, _, _)).Times(1).WillOnce(Return(MAKE_RETURN_CODE(FileSystemIfReturnCode, ITC_FILESYSTEM_OK)));
    EXPECT_CALL(*m_cWrapperIfMock, cPthreadKeyCreate(&m_transportSysvMsgQueue->m_destructKey, &destructRxThreadWrapper)).Times(1).WillOnce(Return(-1));
    
    itc_mailbox_id_t regionId = 0x00100000;
    
    auto rc = m_transportSysvMsgQueue->initialise(regionId, 1, ITC_FLAG_FORCE_REINIT_TRANSPORTS);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_SYSCALL_ERROR));
    ASSERT_EQ(m_transportSysvMsgQueue->m_pid, m_cWrapperIfMock->cGetPid());
    ASSERT_EQ(m_transportSysvMsgQueue->m_regionId, regionId);
    ASSERT_EQ(m_transportSysvMsgQueue->m_isInitialised, false);
}

TEST_F(ItcTransportSysvMsgQueueTest, initialiseTest4)
{
    /***
     * Test scenario: test force re-init transport, happy case.
     */
    m_transportSysvMsgQueue->m_isInitialised = true;
    
    /* releaseAllResources happy case */
    pthread_key_t mockedKey = 99;
    m_transportSysvMsgQueue->m_destructKey = mockedKey;
    EXPECT_CALL(*m_cWrapperIfMock, cPthreadKeyDelete(mockedKey)).Times(1).WillOnce(Return(0));
    
    /* getMaxMessageSize already exist case */
    m_transportSysvMsgQueue->m_maxMsgSize = 1000;
    
    EXPECT_CALL(*m_fileSystemIfMock, createPath(_, _, _, _)).Times(1).WillOnce(Return(MAKE_RETURN_CODE(FileSystemIfReturnCode, ITC_FILESYSTEM_OK)));
    EXPECT_CALL(*m_cWrapperIfMock, cPthreadKeyCreate(&m_transportSysvMsgQueue->m_destructKey, &destructRxThreadWrapper)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*m_threadManagerIfMock, addThread(
    Truly(
        [](const Task &task)
        {
            return task.taskFunc == &sysvMsgQueueRxThreadWrapper && task.taskArgs == nullptr;
        }
    ), _, _)).Times(1).WillOnce(Return(MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_OK)));
    
    itc_mailbox_id_t regionId = 0x00100000;
    
    auto rc = m_transportSysvMsgQueue->initialise(regionId, 1, ITC_FLAG_FORCE_REINIT_TRANSPORTS);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_OK));
    ASSERT_EQ(m_transportSysvMsgQueue->m_pid, m_cWrapperIfMock->cGetPid());
    ASSERT_EQ(m_transportSysvMsgQueue->m_regionId, regionId);
    ASSERT_EQ(m_transportSysvMsgQueue->m_isInitialised, true);
}

TEST_F(ItcTransportSysvMsgQueueTest, releaseTest1)
{
    /***
     * Test scenario: test release a not-initialised-yet sysv msg queue transport.
     */
    m_transportSysvMsgQueue->m_isInitialised = false;
    
    auto rc = m_transportSysvMsgQueue->release();
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_NOT_INITIALISED));
}

TEST_F(ItcTransportSysvMsgQueueTest, releaseTest2)
{
    /***
     * Test scenario: test release all resources, happy case.
     */
    m_transportSysvMsgQueue->m_isInitialised = true;
    
    /* releaseAllResources happy case */
    pthread_key_t mockedKey = 99;
    m_transportSysvMsgQueue->m_destructKey = mockedKey;
    EXPECT_CALL(*m_cWrapperIfMock, cPthreadKeyDelete(mockedKey)).Times(1).WillOnce(Return(0));
    
    auto rc = m_transportSysvMsgQueue->release();
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_OK));
}

TEST_F(ItcTransportSysvMsgQueueTest, sendTest1)
{
    /***
     * Test scenario: test not initialised yet.
     */
    m_transportSysvMsgQueue->m_isInitialised = false;
    MailboxContactInfo toMboxInfo {ITC_MAILBOX_ID_DEFAULT, ITC_MAILBOX_ID_DEFAULT};
    
    auto rc = m_transportSysvMsgQueue->send(nullptr, toMboxInfo);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_NOT_INITIALISED));
}

TEST_F(ItcTransportSysvMsgQueueTest, sendTest2)
{
    /***
     * Test scenario: test send to invalid regionId.
     */
    m_transportSysvMsgQueue->m_isInitialised = true;
    MailboxContactInfo toMboxInfo {ITC_MAILBOX_ID_DEFAULT, 0x11100005};
    
    auto rc = m_transportSysvMsgQueue->send(nullptr, toMboxInfo);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_INVALID_REGION));
}

TEST_F(ItcTransportSysvMsgQueueTest, sendTest3)
{
    /***
     * Test scenario: test first try of sending returns EINTR, next try of sending is successful, but deallocateMessage failed.
     */
    m_transportSysvMsgQueue->m_isInitialised = true;
    itc_mailbox_id_t sender = 0x00100004;
    itc_mailbox_id_t receiver = 0x00100005;
    MailboxContactInfo toMboxInfo {ITC_MAILBOX_ID_DEFAULT, receiver};
    auto adminMsg = reinterpret_cast<ItcAdminMessageRawPtr>(new uint8_t[ITC_ADMIN_MESSAGE_MIN_SIZE]);
    adminMsg->sender = sender;
    adminMsg->receiver = receiver;
    adminMsg->msgno = ITC_MESSAGE_MSGNO_DEFAULT;
    adminMsg->size = ITC_MESSAGE_MSGNO_SIZE;
    int32_t msgQueueId = 4;
    m_transportSysvMsgQueue->m_contactList.at(0x001).msgQueueId = msgQueueId;
    
    {
        InSequence seq;
        EXPECT_CALL(*m_cWrapperIfMock, cMsgsnd(msgQueueId,
        Truly(
            [&adminMsg](const void *arg)
            {
                auto msgp = const_cast<void *>(arg);
                auto txMsgno = reinterpret_cast<long *>(msgp);
                bool isMatched = *txMsgno == ITC_SYSV_MESSAGE_QUEUE_TX_MSGNO;
                auto admMsg = reinterpret_cast<ItcAdminMessageRawPtr>(reinterpret_cast<uint8_t *>(msgp) + sizeof(long));
                isMatched &= admMsg->sender == adminMsg->sender;
                isMatched &= admMsg->receiver == adminMsg->receiver;
                return isMatched;
            }
        ), ITC_ADMIN_MESSAGE_MIN_SIZE, MSG_NOERROR)).Times(1).WillOnce(Return(-1));
        errno = EINTR;
        EXPECT_CALL(*m_cWrapperIfMock, cMsgsnd(_, _, _, _)).Times(1).WillOnce(Return(0));
    }
    
    EXPECT_CALL(*m_itcPlatformIfMock, deallocateMessage(
        Truly(
            [&adminMsg](ItcMessageRawPtr arg)
            {
                auto msg = CONVERT_TO_ADMIN_MESSAGE(arg);
                return msg->sender == adminMsg->sender && msg->receiver == adminMsg->receiver;
            }
        )
    )).Times(1).WillOnce(Return(MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_FAILED)));
    
    auto rc = m_transportSysvMsgQueue->send(adminMsg, toMboxInfo);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_SYSCALL_ERROR));
    delete[] adminMsg;
}

TEST_F(ItcTransportSysvMsgQueueTest, sendTest4)
{
    /***
     * Test scenario: test first try of sending returns EINVAL, next try of sending and deallocateMessage are successful.
     */
    m_transportSysvMsgQueue->m_isInitialised = true;
    itc_mailbox_id_t sender = 0x00100004;
    itc_mailbox_id_t receiver = 0x00100005;
    MailboxContactInfo toMboxInfo {ITC_MAILBOX_ID_DEFAULT, receiver};
    auto adminMsg = reinterpret_cast<ItcAdminMessageRawPtr>(new uint8_t[ITC_ADMIN_MESSAGE_MIN_SIZE]);
    adminMsg->sender = sender;
    adminMsg->receiver = receiver;
    adminMsg->msgno = ITC_MESSAGE_MSGNO_DEFAULT;
    adminMsg->size = ITC_MESSAGE_MSGNO_SIZE;
    int32_t msgQueueId = 4;
    m_transportSysvMsgQueue->m_contactList.at(0x001).msgQueueId = msgQueueId;
    
    {
        InSequence seq;
        EXPECT_CALL(*m_cWrapperIfMock, cMsgsnd(msgQueueId,
        Truly(
            [&adminMsg](const void *arg)
            {
                auto msgp = const_cast<void *>(arg);
                auto txMsgno = reinterpret_cast<long *>(msgp);
                bool isMatched = *txMsgno == ITC_SYSV_MESSAGE_QUEUE_TX_MSGNO;
                auto admMsg = reinterpret_cast<ItcAdminMessageRawPtr>(reinterpret_cast<uint8_t *>(msgp) + sizeof(long));
                isMatched &= admMsg->sender == adminMsg->sender;
                isMatched &= admMsg->receiver == adminMsg->receiver;
                return isMatched;
            }
        ), ITC_ADMIN_MESSAGE_MIN_SIZE, MSG_NOERROR)).Times(1).WillOnce(Return(-1));
        errno = EINVAL;
        EXPECT_CALL(*m_cWrapperIfMock, cMsgsnd(_, _, _, _)).Times(1).WillOnce(Return(0));
    }
    
    /* getMsgQueueId happy case */
    itc_mailbox_id_t newMboxId = receiver & ITC_MASK_REGION_ID;
	itc_mailbox_id_t projectId = newMboxId >> ITC_REGION_ID_SHIFT;
    key_t mockedKey = 4;
    int32_t mockedMsgQueueId = 10;
    EXPECT_CALL(*m_cWrapperIfMock, cFtok(
    Truly(
        [](const char *pathname)
        {
            return std::string(pathname) == ITC_PATH_SYSVMQ_FILE_NAME;
        }
    ), projectId)).Times(1).WillOnce(Return(mockedKey));
    EXPECT_CALL(*m_cWrapperIfMock, cMsgGet(mockedKey, 0)).Times(1).WillOnce(Return(mockedMsgQueueId));
    
    EXPECT_CALL(*m_itcPlatformIfMock, deallocateMessage(
        Truly(
            [&adminMsg](ItcMessageRawPtr arg)
            {
                auto msg = CONVERT_TO_ADMIN_MESSAGE(arg);
                return msg->sender == adminMsg->sender && msg->receiver == adminMsg->receiver;
            }
        )
    )).Times(1).WillOnce(Return(MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_OK)));
    
    auto rc = m_transportSysvMsgQueue->send(adminMsg, toMboxInfo);
    ASSERT_EQ(rc, MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_OK));
    delete[] adminMsg;
}

TEST_F(ItcTransportSysvMsgQueueTest, destructRxThreadTest1)
{
    /***
     * Test scenario: test failed to msgctl.
     */
    int32_t mockedMsgQueueId = 1;
    m_transportSysvMsgQueue->m_msgQueueId = mockedMsgQueueId;
    
    EXPECT_CALL(*m_cWrapperIfMock, cMsgctl(mockedMsgQueueId, IPC_RMID, nullptr)).Times(1).WillOnce(Return(-1));
    
    m_transportSysvMsgQueue->destructRxThread(nullptr);
    ASSERT_EQ(m_transportSysvMsgQueue->m_isInitialised, false);
    ASSERT_EQ(m_transportSysvMsgQueue->m_isTerminated, true);
    ASSERT_EQ(m_transportSysvMsgQueue->m_msgQueueId, -1);
}

TEST_F(ItcTransportSysvMsgQueueTest, sysvMsgQueueRxThreadTest1)
{
    /***
     * Test scenario: test failed to msgctl.
     */
    itc_mailbox_id_t mboxId = 0x00100005;
    itc_mailbox_id_t regionID = mboxId & ITC_MASK_REGION_ID;
    m_transportSysvMsgQueue->m_regionId = regionID;
    char mboxName[30];
	::sprintf(mboxName, "itc_rx_sysvmq_0x%08x", regionID);
    pthread_key_t mockedKey = 4;
    m_transportSysvMsgQueue->m_destructKey = mockedKey;
    
    EXPECT_CALL(*m_itcPlatformIfMock, createMailbox(
        Truly(
            [&mboxName](const std::string &name)
            {
                return name == std::string(mboxName);
            }
        )    
    , ITC_FLAG_DEFAULT)).Times(1).WillOnce(Return(mboxId));
    EXPECT_CALL(*m_cWrapperIfMock, cPthreadSetSpecific(mockedKey, reinterpret_cast<void *>(mboxId))).Times(1).WillOnce(Return(-1));
    
    m_transportSysvMsgQueue->sysvMsgQueueRxThread(nullptr);
    ASSERT_EQ(m_transportSysvMsgQueue->m_msgQueueId, -1);
}

TEST_F(ItcTransportSysvMsgQueueTest, sysvMsgQueueRxThreadTest2)
{
    /***
     * Test scenario: test failures on second msg queue setup.
     */
    itc_mailbox_id_t mboxId = 0x00100005;
    itc_mailbox_id_t regionID = mboxId & ITC_MASK_REGION_ID;
    m_transportSysvMsgQueue->m_regionId = regionID;
    char mboxName[30];
	::sprintf(mboxName, "itc_rx_sysvmq_0x%08x", regionID);
    pthread_key_t mockedKey = 4;
    m_transportSysvMsgQueue->m_destructKey = mockedKey;
    
    InSequence seq;
    EXPECT_CALL(*m_itcPlatformIfMock, createMailbox(
        Truly(
            [&mboxName](const std::string &name)
            {
                return name == std::string(mboxName);
            }
        )    
    , ITC_FLAG_DEFAULT)).Times(1).WillOnce(Return(mboxId));
    EXPECT_CALL(*m_cWrapperIfMock, cPthreadSetSpecific(mockedKey, reinterpret_cast<void *>(mboxId))).Times(1).WillOnce(Return(0));
    
    int32_t projectId = (m_transportSysvMsgQueue->m_regionId >> ITC_REGION_ID_SHIFT);
    key_t key = 1;
    EXPECT_CALL(*m_cWrapperIfMock, cFtok(
        Truly(
            [](const char *pathname)
            {
                return std::string(pathname) == ITC_PATH_SYSVMQ_FILE_NAME;
            }
    ), projectId)).Times(1).WillOnce(Return(key));
    int32_t msgQueueId = 1;
    EXPECT_CALL(*m_cWrapperIfMock, cMsgGet(key, IPC_CREAT | 0666)).Times(1).WillOnce(Return(msgQueueId));
    EXPECT_CALL(*m_cWrapperIfMock, cMsgctl(msgQueueId, IPC_STAT, _)).Times(1).WillOnce(DoAll(
        WithArg<2>(
            [](msqid_ds *arg)
            {
                arg->msg_qnum = 1;
            }
        ), Return(0)
    ));
    EXPECT_CALL(*m_cWrapperIfMock, cMsgctl(msgQueueId, IPC_RMID, _)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*m_cWrapperIfMock, cMsgGet(key, IPC_CREAT | 0666)).Times(1).WillOnce(Return(-1));
    
    m_transportSysvMsgQueue->sysvMsgQueueRxThread(nullptr);
    ASSERT_EQ(m_transportSysvMsgQueue->m_msgQueueId, -1);
    ASSERT_EQ(m_transportSysvMsgQueue->m_rxBuffer, nullptr);
}

TEST_F(ItcTransportSysvMsgQueueTest, sysvMsgQueueRxThreadTest3)
{
    /***
     * Test scenario: test negative rxLen received.
     */
    itc_mailbox_id_t mboxId = 0x00100005;
    itc_mailbox_id_t regionID = mboxId & ITC_MASK_REGION_ID;
    m_transportSysvMsgQueue->m_regionId = regionID;
    char mboxName[30];
	::sprintf(mboxName, "itc_rx_sysvmq_0x%08x", regionID);
    pthread_key_t mockedKey = 4;
    m_transportSysvMsgQueue->m_destructKey = mockedKey;
    
    InSequence seq;
    EXPECT_CALL(*m_itcPlatformIfMock, createMailbox(
        Truly(
            [&mboxName](const std::string &name)
            {
                return name == std::string(mboxName);
            }
        )    
    , ITC_FLAG_DEFAULT)).Times(1).WillOnce(Return(mboxId));
    EXPECT_CALL(*m_cWrapperIfMock, cPthreadSetSpecific(mockedKey, reinterpret_cast<void *>(mboxId))).Times(1).WillOnce(Return(0));
    
    int32_t projectId = (m_transportSysvMsgQueue->m_regionId >> ITC_REGION_ID_SHIFT);
    key_t key = 1;
    EXPECT_CALL(*m_cWrapperIfMock, cFtok(
        Truly(
            [](const char *pathname)
            {
                return std::string(pathname) == ITC_PATH_SYSVMQ_FILE_NAME;
            }
    ), projectId)).Times(1).WillOnce(Return(key));
    int32_t msgQueueId = 1;
    EXPECT_CALL(*m_cWrapperIfMock, cMsgGet(key, IPC_CREAT | 0666)).Times(1).WillOnce(Return(msgQueueId));
    EXPECT_CALL(*m_cWrapperIfMock, cMsgctl(msgQueueId, IPC_STAT, _)).Times(1).WillOnce(DoAll(
        WithArg<2>(
            [](msqid_ds *arg)
            {
                arg->msg_qnum = 1;
            }
        ), Return(0)
    ));
    EXPECT_CALL(*m_cWrapperIfMock, cMsgctl(msgQueueId, IPC_RMID, _)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*m_cWrapperIfMock, cMsgGet(key, IPC_CREAT | 0666)).Times(1).WillOnce(Return(msgQueueId));
    
    /* getMaxMessageSize simulate already exists case */
    size_t maxMsgSize = 1000;
    m_transportSysvMsgQueue->m_maxMsgSize = maxMsgSize;
    
    m_transportSysvMsgQueue->m_syncObj = std::make_shared<SyncObject>();
    
    EXPECT_CALL(*m_cWrapperIfMock, cMsgrcv(msgQueueId, _, maxMsgSize - sizeof(long), 0, 0)).Times(1).WillOnce(Return(-1));
    auto &isTerminated = m_transportSysvMsgQueue->m_isTerminated;
    EXPECT_CALL(*m_cWrapperIfMock, cMsgrcv(_, _, _, _,
        Truly(
            [&isTerminated](int32_t msgflg)
            {
                isTerminated = true;
                return msgflg == 0;
            }
        )
    )).Times(1).WillOnce(Return(-1));
    
    
    m_transportSysvMsgQueue->sysvMsgQueueRxThread(nullptr);
    ASSERT_EQ(m_transportSysvMsgQueue->m_msgQueueId, msgQueueId);
}


} // namespace INTERNAL
} // namespace ITC