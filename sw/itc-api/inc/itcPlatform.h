#pragma once

#include "itc.h"
#include "itcSystemProto.h"
#include "itcMailbox.h"
#include "itcAdminMessage.h"
#include "itcConstant.h"
#include "itcConcurrentContainer.h"

#include <mutex>
#include <memory>
#include <cstdint>
#include <queue>
#include <array>
#include <unordered_map>
#include <gtest/gtest.h>

#include <pthread.h>

// using namespace CommonUtils::V1::EnumUtils;

void destructMailboxAtThreadExitWrapper(void *args);

namespace ITC
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

using namespace ITC::PROVIDED;

class ItcPlatform : public ItcPlatformIf
{
public:
	static std::weak_ptr<ItcPlatform> getInstance();
	
	ItcPlatformIfReturnCode initialise(uint32_t flags = ITC_FLAG_DEFAULT) override;
	ItcPlatformIfReturnCode release() override;
	ItcMessageRawPtr allocateMessage(uint32_t msgno, size_t size = ITC_MESSAGE_MSGNO_SIZE) override;
	ItcPlatformIfReturnCode deallocateMessage(ItcMessageRawPtr msg) override;
	itc_mailbox_id_t createMailbox(const std::string &name, uint32_t flags = ITC_FLAG_DEFAULT) override;
	ItcPlatformIfReturnCode deleteMailbox(itc_mailbox_id_t mboxId) override;
	ItcPlatformIfReturnCode send(ItcMessageRawPtr msg, const MailboxContactInfo &toMbox) override;
	ItcMessageRawPtr receive(uint32_t mode = ITC_MODE_DEFAULT) override;
	MailboxContactInfo locateMailboxSync(const std::string &mboxName, uint32_t mode = ITC_MODE_LOCATE_IN_ALL, uint32_t timeout = 0) override;
	ItcPlatformIfReturnCode locateMailboxAsync(const std::string &mboxName, uint32_t mode = ITC_MODE_LOCATE_IN_ALL) override;
	
	itc_mailbox_id_t getSender(const ItcMessageRawPtr &msg) override;
	itc_mailbox_id_t getReceiver(const ItcMessageRawPtr &msg) override;
	size_t getMsgSize(const ItcMessageRawPtr &msg) override;
	int32_t myMailboxFd() override;
	std::string getMailboxName(itc_mailbox_id_t mboxId) override;

	ItcPlatform();
	virtual ~ItcPlatform();

	// First prevent copy/move construtors/assignments
	ItcPlatform(const ItcPlatform&)               = delete;
	ItcPlatform(ItcPlatform&&)                    = delete;
	ItcPlatform& operator=(const ItcPlatform&)    = delete;
	ItcPlatform& operator=(ItcPlatform&&)         = delete;

private:
	bool startDaemon(const std::string &programPath);
	bool checkAndStartItcServer();
	void destructMailboxAtThreadExit(void *args);
	ItcPlatformIfReturnCode forwardMessageToItcServer(ItcAdminMessageRawPtr adminMsg, itc_mailbox_id_t toWorldId);

private:
	SINGLETON_DECLARATION(ItcPlatform)
	
	itc_mailbox_id_t m_regionId {ITC_MAILBOX_ID_DEFAULT};
	itc_mailbox_id_t m_itcServerMboxId {ITC_MAILBOX_ID_DEFAULT};
	std::shared_ptr<ConcurrentContainer<ItcMailbox, ITC_MAX_SUPPORTED_MAILBOXES>> m_mboxList {nullptr};
	pthread_key_t m_destructKey;
	bool m_isInitialised {false};
	static thread_local ItcMailboxRawPtr m_myMailbox;
	
	friend void ::destructMailboxAtThreadExitWrapper(void *args);
	
	friend class ItcPlatformIfTest;
	FRIEND_TEST(ItcPlatformIfTest, checkAndStartItcServerTest1);

}; // class ItcPlatform

} // namespace INTERNAL
} // namespace ITC
