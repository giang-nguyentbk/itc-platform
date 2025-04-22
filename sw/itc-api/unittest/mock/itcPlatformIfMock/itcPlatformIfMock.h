#pragma once

#include "itc.h"
#include "itcMutex.h"
#include "itcConstant.h"

#include <gmock/gmock.h>

namespace ITC
{
namespace INTERNAL
{
using namespace ITC::PROVIDED;

class ItcPlatformIfMock : public ItcPlatformIf
{
public:
    static std::weak_ptr<ItcPlatformIfMock> getInstance();
    virtual ~ItcPlatformIfMock() = default;

    MOCK_METHOD(ItcPlatformIfReturnCode, initialise, (uint32_t flags), (override));
    MOCK_METHOD(ItcPlatformIfReturnCode, release, (), (override));
    MOCK_METHOD(ItcMessageRawPtr, allocateMessage, (uint32_t msgno, size_t size), (override));
    MOCK_METHOD(ItcPlatformIfReturnCode, deallocateMessage, (ItcMessageRawPtr msg), (override));
    MOCK_METHOD(itc_mailbox_id_t, createMailbox, (const std::string &name, uint32_t flags), (override));
    MOCK_METHOD(ItcPlatformIfReturnCode, deleteMailbox, (itc_mailbox_id_t mboxId), (override));
    MOCK_METHOD(ItcPlatformIfReturnCode, send, (ItcMessageRawPtr msg, const MailboxContactInfo &toMbox), (override));
    MOCK_METHOD(ItcMessageRawPtr, receive, (uint32_t mode), (override));
    MOCK_METHOD(MailboxContactInfo, locateMailboxSync, (const std::string &mboxName, uint32_t mode, uint32_t timeout), (override));
    MOCK_METHOD(ItcPlatformIfReturnCode, locateMailboxAsync, (const std::string &mboxName, uint32_t mode), (override));
    MOCK_METHOD(itc_mailbox_id_t, getSender, (const ItcMessageRawPtr &msg), (override));
    MOCK_METHOD(itc_mailbox_id_t, getReceiver, (const ItcMessageRawPtr &msg), (override));
    MOCK_METHOD(size_t, getMsgSize, (const ItcMessageRawPtr &msg), (override));
    MOCK_METHOD(int32_t, myMailboxFd, (), (override));
    MOCK_METHOD(std::string, getMailboxName, (itc_mailbox_id_t mboxId), (override));

private:
    ItcPlatformIfMock() = default;

private:
    SINGLETON_DECLARATION(ItcPlatformIfMock)
    
    friend class ItcTransportLocalTest;
    friend class ItcTransportLSocketTest;
    friend class ItcTransportSysvMsgQueueTest;
    
}; // class ItcPlatformIfMock

} // namespace INTERNAL
} // namespace ITC