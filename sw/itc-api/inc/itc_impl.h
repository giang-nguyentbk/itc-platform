#pragma once

#include "itc.h"
#include "itcSystemProto.h"
#include "itcMailbox.h"
#include "itcAdminMessage.h"

#include <mutex>
#include <memory>
#include <cstdint>
#include <queue>

// using namespace CommonUtils::V1::EnumUtils;


namespace ItcPlatform
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

using namespace ItcPlatform::PROVIDED;

class ItcPlatformImpl : public ItcPlatformIf
{
public:
	static std::weak_ptr<ItcPlatformImpl> getInstance();
	
	ItcPlatformIfReturnCode initialise(uint32_t mboxCount, uint32_t flags) override;
	ItcPlatformIfReturnCode release() override;
	ItcMessageRawPtr createMessage(size_t size, uint32_t msgno) override;
	ItcPlatformIfReturnCode deleteMessage(ItcMessageRawPtr msg) override;
	itc_mailbox_id_t createMailbox(const std::string &name, uint32_t flags) override;
	ItcPlatformIfReturnCode deleteMailbox(itc_mailbox_id_t mboxId) override;
	ItcPlatformIfReturnCode sendMessage(ItcMessageRawPtr msg, const MailboxContactInfo &toMbox, uint32_t mode) override;
	ItcMessageRawPtr receiveMessage(uint32_t mode, uint32_t timeout) override;
	MailboxContactInfo locateMailboxSync(const std::string &mboxName, uint32_t timeout, uint32_t mode) override;
	ItcPlatformIfReturnCode locateMailboxAsync(const std::string &mboxName, uint32_t timeout, uint32_t mode) override;
	
	itc_mailbox_id_t getMyMailboxId() override;
	itc_mailbox_id_t getSenderMailboxId(const ItcMessageRawPtr &msg) override;
	itc_mailbox_id_t getReceiverMailboxId(const ItcMessageRawPtr &msg) override;
	size_t getMessageSize(const ItcMessageRawPtr &msg) override;
	int32_t getMyMailboxFd() override;
	std::string getMailboxName(itc_mailbox_id_t mboxId) override;

	ItcPlatformImpl();
	virtual ~ItcPlatformImpl();

	// First prevent copy/move construtors/assignments
	ItcPlatformImpl(const ItcPlatformImpl&)               = delete;
	ItcPlatformImpl(ItcPlatformImpl&&)                    = delete;
	ItcPlatformImpl& operator=(const ItcPlatformImpl&)    = delete;
	ItcPlatformImpl& operator=(ItcPlatformImpl&&)         = delete;

private:
	static std::shared_ptr<ItcPlatformImpl> m_instance;
	static std::mutex m_singletonMutex;
	std::queue<size_t /* Mailbox index in std::vector */> m_availMboxIndexQueue;
	uint32_t m_regionId;

}; // class ItcPlatformImpl

} // namespace INTERNAL
} // namespace ItcPlatform
