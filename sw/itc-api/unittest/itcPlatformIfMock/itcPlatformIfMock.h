#pragma once

#include "itc.h"
#include "itcMutex.h"

#include <gmock/gmock.h>

namespace ItcPlatform
{
namespace INTERNAL
{
using namespace ItcPlatform::PROVIDED;

class ItcPlatformIfMock : public ItcPlatformIf
{
public:
    static std::weak_ptr<ItcPlatformIfMock> getInstance();
    virtual ~ItcPlatformIfMock() = default;

    MOCK_METHOD(ItcPlatformIfReturnCode, initialise, (uint32_t mboxCount, uint32_t flags), (override));
    MOCK_METHOD(ItcPlatformIfReturnCode, release, (), (override));
    MOCK_METHOD(ItcMessageRawPtr, createMessage, (size_t size, uint32_t msgno), (override));
    MOCK_METHOD(ItcPlatformIfReturnCode, deleteMessage, (ItcMessageRawPtr msg), (override));
    MOCK_METHOD(itc_mailbox_id_t, createMailbox, (const std::string &name, uint32_t flags), (override));
    MOCK_METHOD(ItcPlatformIfReturnCode, deleteMailbox, (itc_mailbox_id_t mboxId), (override));
    MOCK_METHOD(ItcPlatformIfReturnCode, sendMessage, (ItcMessageRawPtr msg, const MailboxContactInfo &toMbox, uint32_t mode), (override));
    MOCK_METHOD(ItcMessageRawPtr, receiveMessage, (uint32_t timeout, uint32_t mode), (override));
    MOCK_METHOD(MailboxContactInfo, locateMailboxSync, (const std::string &mboxName, uint32_t timeout, uint32_t mode), (override));
    MOCK_METHOD(ItcPlatformIfReturnCode, locateMailboxAsync, (const std::string &mboxName, uint32_t timeout, uint32_t mode), (override));
    MOCK_METHOD(itc_mailbox_id_t, getMyMailboxId, (), (override));
    MOCK_METHOD(itc_mailbox_id_t, getSenderMailboxId, (const ItcMessageRawPtr &msg), (override));
    MOCK_METHOD(itc_mailbox_id_t, getReceiverMailboxId, (const ItcMessageRawPtr &msg), (override));
    MOCK_METHOD(size_t, getMessageSize, (const ItcMessageRawPtr &msg), (override));
    MOCK_METHOD(int32_t, getMyMailboxFd, (), (override));
    MOCK_METHOD(std::string, getMailboxName, (itc_mailbox_id_t mboxId), (override));

private:
    ItcPlatformIfMock() = default;

private:
    static std::shared_ptr<ItcPlatformIfMock> m_instance;
	static std::mutex m_singletonMutex;
    
    friend class ItcTransportLocalTest;
    friend class ItcTransportLSocketTest;
    friend class ItcTransportSysvMsgQueueTest;
    
}; // class ItcPlatformIfMock

} // namespace INTERNAL
} // namespace ItcPlatform