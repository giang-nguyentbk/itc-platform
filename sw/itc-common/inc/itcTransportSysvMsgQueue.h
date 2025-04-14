#pragma once

#include <string>
#include <array>
#include <limits>
#include <cstdint>
#include <cstring>
#include <cstddef>
#include <mutex>
#include <memory>

#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>

// #include <enumUtils.h>

#include "itc.h"
#include "itcConstant.h"
#include "itcMailbox.h"
#include "itcAdminMessage.h"
#include "itcThreadManagerIf.h"
#include "itcMutex.h"
#include "itcCWrapperIf.h"

void destructRxThreadWrapper(void *args);
void *sysvMsgQueueRxThreadWrapper(void *args);

namespace ITC
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

using namespace ITC::PROVIDED;

#define ITC_SYSV_MESSAGE_QUEUE_TX_MSGNO         (uint32_t)(0x1)
#define ITC_PATH_SYSVMQ_FILE_NAME               "/tmp/itc/sysvmsq/sysvmq-file"

struct SysvMsgQueueContactInfo
{
	itc_mailbox_id_t    regionId {ITC_MAILBOX_ID_DEFAULT};
	int32_t		        msgQueueId {-1};
};

/***
 * This transport is to exchange messages between Regions/Processes.
 */
class ItcTransportSysvMsgQueue
{
public:
    static std::weak_ptr<ItcTransportSysvMsgQueue> getInstance();
    virtual ~ItcTransportSysvMsgQueue() = default;
    
    ItcTransportSysvMsgQueue(const ItcTransportSysvMsgQueue &other) = delete;
    ItcTransportSysvMsgQueue &operator=(const ItcTransportSysvMsgQueue &other) = delete;
    ItcTransportSysvMsgQueue(ItcTransportSysvMsgQueue &&other) noexcept = delete;
    ItcTransportSysvMsgQueue &operator=(ItcTransportSysvMsgQueue &&other) noexcept = delete;
    
    bool initialise(itc_mailbox_id_t regionId = ITC_MAILBOX_ID_DEFAULT);
    void release();
    ItcPlatformIfReturnCode send(ItcAdminMessageRawPtr adminMsg);
    
private:
    ItcTransportSysvMsgQueue() = default;
    
    size_t getMaxMessageSize();
    int32_t getMsgQueueId(itc_mailbox_id_t mboxId);
    void removeContactInfoAtIndex(size_t atIndex);
    void addContactInfoAtIndex(size_t atIndex, itc_mailbox_id_t mboxId);
    bool parseAndForwardMessage(uint8_t *rxBuffer, ssize_t length);
    void destructRxThread(void *args);
    void *sysvMsgQueueRxThread(void *args);
    
private:
    SINGLETON_DECLARATION(ItcTransportSysvMsgQueue)
    
    itc_mailbox_id_t m_regionId {ITC_MAILBOX_ID_DEFAULT};
    itc_mailbox_id_t m_mboxId {ITC_MAILBOX_ID_DEFAULT};
    int32_t m_msgQueueId {-1};
    pid_t m_pid {-1};
    std::shared_ptr<SyncObject> m_syncObj;
	pthread_key_t m_destructKey;
    bool m_isInitialised {false};
    bool m_isRxThreadTerminated {false};
    size_t m_maxMsgSize {std::numeric_limits<size_t>::max()};
    uint8_t *m_rxBuffer {nullptr};
    std::array<SysvMsgQueueContactInfo, ITC_MAX_SUPPORTED_REGIONS> m_contactList;
    
    friend void ::destructRxThreadWrapper(void *args);
    friend void *::sysvMsgQueueRxThreadWrapper(void *args);
    
    friend class ItcTransportSysvMsgQueueTest;
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, getMsgQueueIdTest1);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, getMsgQueueIdTest2);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, getMsgQueueIdTest3);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, parseAndForwardMessageTest1);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, parseAndForwardMessageTest2);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, parseAndForwardMessageTest3);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, parseAndForwardMessageTest4);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, parseAndForwardMessageTest5);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, getMaxMessageSizeTest1);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, getMaxMessageSizeTest2);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, getMaxMessageSizeTest3);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, initialiseTest1);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, initialiseTest2);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, initialiseTest3);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, initialiseTest4);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, releaseTest1);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, releaseTest2);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, sendTest1);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, sendTest2);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, sendTest3);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, sendTest4);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, destructRxThreadTest1);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, sysvMsgQueueRxThreadTest1);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, sysvMsgQueueRxThreadTest2);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, sysvMsgQueueRxThreadTest3);
}; // class ItcTransportSysvMsgQueue

} // namespace INTERNAL
} // namespace ITC