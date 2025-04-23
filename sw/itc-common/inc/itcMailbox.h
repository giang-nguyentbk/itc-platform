#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <queue>
#include <functional>

// #include <enumUtils.h>

#include "itc.h"
#include "itcConstant.h"
#include "itcSyncObject.h"
#include "itcCWrapperIf.h"
#include "itcMutex.h"
#include "itcLockFreeQueue.h"

#include <gtest/gtest.h>

namespace ITC
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

using namespace ITC::PROVIDED;

#define ITC_FLAG_MAILBOX_IN_RX      			(uint32_t)(0x1)
#define ITC_MAILBOX_RX_MESSAGE_QUEUE_SIZE      	(uint32_t)(1024)

/* To enable lock-free data structure, never make sizeof(ItcMailbox) > 64 bytes. */
class ItcMailbox
{
public:
	ItcMailbox(uint32_t flags = ITC_FLAG_DEFAULT)
		:  m_flags(flags)
	{
		m_rxMsgQueue = std::make_unique<LockFreeQueue<ItcAdminMessageRawPtr, ITC_MAILBOX_RX_MESSAGE_QUEUE_SIZE, nullptr, true, true, false, false>>();
	}
	
	~ItcMailbox()
	{
		setState(false);
	}
	
	ItcMailbox(const ItcMailbox &other)
	{
		m_mailboxId = other.m_mailboxId;
		m_flags = other.m_flags;
		m_rxMsgQueue = std::make_unique<LockFreeQueue<ItcAdminMessageRawPtr, ITC_MAILBOX_RX_MESSAGE_QUEUE_SIZE, nullptr, true, true, false, false>>();
	}
	ItcMailbox &operator=(const ItcMailbox &other)
	{
		m_mailboxId = other.m_mailboxId;
		m_flags = other.m_flags;
		m_rxMsgQueue = std::make_unique<LockFreeQueue<ItcAdminMessageRawPtr, ITC_MAILBOX_RX_MESSAGE_QUEUE_SIZE, nullptr, true, true, false, false>>();
		return *this;
	}
	ItcMailbox(ItcMailbox &&other) noexcept
	{
		if(this != &other)
		{
			m_mailboxId = std::move(other.m_mailboxId);
			m_flags = std::move(other.m_flags);
			m_rxMsgQueue = std::move(other.m_rxMsgQueue);
		}
	}
	ItcMailbox &operator=(ItcMailbox &&other) noexcept
	{
		if(this != &other)
		{
			setState(false);
			m_mailboxId = std::move(other.m_mailboxId);
			m_flags = std::move(other.m_flags);
			m_rxMsgQueue = std::move(other.m_rxMsgQueue);
		}
		return *this;
	}
	
	bool operator==(const ItcMailbox& other) const
	{
        return m_mailboxId == other.m_mailboxId && m_flags == other.m_flags;
    }
	
	bool operator!=(const ItcMailbox& other) const
	{
        return !(*this == other);
    }
	
	bool push(ItcAdminMessageRawPtr msg);
	/* Default is blocking mode. */
	ItcAdminMessageRawPtr pop(uint32_t mode = ITC_MODE_DEFAULT);
	void setState(bool newState);
	
public:
	itc_mailbox_id_t m_mailboxId {ITC_MAILBOX_ID_DEFAULT};
    uint32_t m_flags {ITC_FLAG_DEFAULT};
	
private:
	std::unique_ptr<LockFreeQueue<ItcAdminMessageRawPtr, ITC_MAILBOX_RX_MESSAGE_QUEUE_SIZE, nullptr, true, true, false, false>> m_rxMsgQueue {nullptr};
	std::atomic_bool m_isActive {false};
	
	friend class ItcMailboxTest;
	FRIEND_TEST(ItcMailboxTest, test1);
	FRIEND_TEST(ItcMailboxTest, test2);
	FRIEND_TEST(ItcMailboxTest, test3);
	FRIEND_TEST(ItcMailboxTest, test4);
	
	friend class ItcTransportLocalTest;
	FRIEND_TEST(ItcTransportLocalTest, test1);
	FRIEND_TEST(ItcTransportLocalTest, test2);
	FRIEND_TEST(ItcTransportLocalTest, sendReceiveTest2);
	FRIEND_TEST(ItcTransportLocalTest, sendReceiveTest3);
	FRIEND_TEST(ItcTransportLocalTest, sendReceiveTest4);
};

using ItcMailboxRawPtr = ItcMailbox *;

} // namespace INTERNAL
} // namespace ITC