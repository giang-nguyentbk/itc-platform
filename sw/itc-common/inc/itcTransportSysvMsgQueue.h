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
#include "itcThreadManager.h"
#include "itcTransportIf.h"
#include "itcMutex.h"
#include "itcCWrapperIf.h"

void destructRxThreadWrapper(void *args);
void *sysvMsgQueueRxThreadWrapper(void *args);

namespace ItcPlatform
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

using namespace ItcPlatform::PROVIDED;

static constexpr uint32_t ITC_MAX_SUPPORTED_REGIONS             = 255;
static constexpr uint32_t ITC_SYSV_MESSAGE_QUEUE_TX_MSGNO       = 0x1;
static const std::string ITC_PATH_SYSVMQ_FILE_NAME          {"/tmp/itc/sysvmsq/sysvmq-file"};

struct SysvMsgQueueContactInfo
{
	itc_mailbox_id_t    regionId {ITC_NO_MAILBOX_ID};
	int32_t		        msgQueueId {-1};
};

/***
 * This transport is to exchange messages between Regions/Processes.
 */
class ItcTransportSysvMsgQueue : public ItcTransportIf
{
public:
    static std::weak_ptr<ItcTransportSysvMsgQueue> getInstance();
    virtual ~ItcTransportSysvMsgQueue() = default;
    
    ItcTransportSysvMsgQueue(const ItcTransportSysvMsgQueue &other) = delete;
    ItcTransportSysvMsgQueue &operator=(const ItcTransportSysvMsgQueue &other) = delete;
    ItcTransportSysvMsgQueue(ItcTransportSysvMsgQueue &&other) noexcept = delete;
    ItcTransportSysvMsgQueue &operator=(ItcTransportSysvMsgQueue &&other) noexcept = delete;
    
    ItcTransportIfReturnCode initialise(itc_mailbox_id_t regionId = ITC_NO_MAILBOX_ID, uint32_t mboxCount = 0, uint32_t flags = ITC_NO_MAILBOX_ID) override;
    ItcTransportIfReturnCode release() override;
    ItcTransportIfReturnCode locateItcServer(itc_mailbox_id_t *assignedRegionId, itc_mailbox_id_t *locatedItcServerMboxId) override;
    ItcTransportIfReturnCode createMailbox(ItcMailboxRawPtr newMbox) override;
    ItcTransportIfReturnCode deleteMailbox(ItcMailboxRawPtr mbox) override;
    ItcTransportIfReturnCode sendMessage(ItcAdminMessageRawPtr adminMsg, const MailboxContactInfo &toMbox) override;
    ItcAdminMessageRawPtr receiveMessage(ItcMailboxRawPtr myMbox) override;
    size_t getMaxMessageSize() override;
    
private:
    ItcTransportSysvMsgQueue() = default;
    
    ItcTransportIfReturnCode releaseAllResources();
    int32_t getMsgQueueId(itc_mailbox_id_t mboxId);
    void removeContactInfoAtIndex(size_t atIndex);
    void addContactInfoAtIndex(size_t atIndex, itc_mailbox_id_t mboxId);
    int32_t parseAndForwardMessage(uint8_t *rxBuffer, ssize_t length);
    void destructRxThread(void *args);
    void *sysvMsgQueueRxThread(void *args);
    
private:
    static std::shared_ptr<ItcTransportSysvMsgQueue> m_instance;
	static std::mutex m_singletonMutex;
    itc_mailbox_id_t m_regionId {ITC_NO_MAILBOX_ID};
    itc_mailbox_id_t m_mboxId {ITC_NO_MAILBOX_ID};
    int32_t m_msgQueueId {-1};
    pid_t m_pid {-1};
    std::shared_ptr<Signal> m_signal;
	pthread_key_t m_destructKey;
    bool m_isInitialised {false};
    bool m_isTerminated {false};
    size_t m_maxMsgSize {std::numeric_limits<size_t>::max()};
    uint8_t *m_rxBuffer {nullptr};
    std::array<SysvMsgQueueContactInfo, ITC_MAX_SUPPORTED_REGIONS> m_contactList;
    
    friend void ::destructRxThreadWrapper(void *args);
    friend void *::sysvMsgQueueRxThreadWrapper(void *args);
    friend class ItcTransportSysvMsgQueueTest;
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, releaseAllResourcesTest1);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, releaseAllResourcesTest2);
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
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, sendMessageTest1);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, sendMessageTest2);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, sendMessageTest3);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, sendMessageTest4);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, destructRxThreadTest1);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, sysvMsgQueueRxThreadTest1);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, sysvMsgQueueRxThreadTest2);
	FRIEND_TEST(ItcTransportSysvMsgQueueTest, sysvMsgQueueRxThreadTest3);
}; // class ItcTransportSysvMsgQueue

} // namespace INTERNAL
} // namespace ItcPlatform