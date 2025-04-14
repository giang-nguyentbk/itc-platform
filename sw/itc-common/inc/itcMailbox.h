#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <queue>

// #include <enumUtils.h>

#include "itc.h"
#include "itcConstant.h"
#include "itcSyncObject.h"
#include "itcCWrapperIf.h"
#include "itcMutex.h"

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

#define ITC_FLAG_MAILBOX_IN_RX      (uint32_t)(0x1);

class ItcMailbox
{
public:
	ItcMailbox(const std::string &iName, uint32_t iFlags = ITC_FLAG_DEFAULT)
		: name(iName),
		  flags(iFlags)
	{
		m_syncObj = std::make_shared<SyncObject>([](SyncObjectElementsSharedPtr elemsPtr)
		{
			auto ret = CWrapperIf::getInstance().lock()->cPthreadCondAttrSetClock(&elemsPtr->condAttrs, CLOCK_MONOTONIC);
			if(ret != 0) UNLIKELY
			{
				TPT_TRACE(TRACE_ERROR, SSTR("Failed to pthread_condattr_setclock, error code = ", ret));
				return -1;
			}
			
			ret = CWrapperIf::getInstance().lock()->cPthreadMutexAttrSetType(&elemsPtr->mtxAttrs, PTHREAD_MUTEX_ERRORCHECK);
			if(ret != 0) UNLIKELY
			{
				TPT_TRACE(TRACE_ERROR, SSTR("Failed to pthread_condattr_setclock, error code = ", ret));
				return -1;
			}
			
			return 0;
		});
	}
	
	~ItcMailbox()
	{
		reset();
	}
	
	ItcMailbox(const ItcMailbox &other) = default;
	ItcMailbox &operator=(const ItcMailbox &other) = default;
	ItcMailbox(ItcMailbox &&other) noexcept
	{
		if(this != &other)
		{
			name = std::move(other.name);
			mailboxId = std::move(other.mailboxId);
			flags = std::move(other.flags);
			m_syncObj = std::move(other.m_syncObj);
			m_rxMsgQueue = std::move(other.m_rxMsgQueue);
		}
	}
	ItcMailbox &operator=(ItcMailbox &&other) noexcept
	{
		if(this != &other)
		{
			reset();
			name = std::move(other.name);
			mailboxId = std::move(other.mailboxId);
			flags = std::move(other.flags);
			m_syncObj = std::move(other.m_syncObj);
			m_rxMsgQueue = std::move(other.m_rxMsgQueue);
		}
		return *this;
	}
	
	void enqueueAndNotify(ItcAdminMessageRawPtr msg);
	ItcAdminMessageRawPtr dequeue(uint32_t mode = ITC_MODE_TIMEOUT_WAIT_FOREVER, uint32_t timeout = 0);
	int32_t getMboxFd();
	void reset();
	
private:
	void notify();
	void push(ItcAdminMessageRawPtr msg);
	ItcAdminMessageRawPtr pop();
	
public:
	std::string     						name;
    itc_mailbox_id_t   						mailboxId {ITC_MAILBOX_ID_DEFAULT};
    uint32_t        						flags {ITC_FLAG_DEFAULT};
	
private:
	std::shared_ptr<SyncObject> 			m_syncObj {nullptr};
	std::queue<ItcAdminMessageRawPtr> 		m_rxMsgQueue;
	
	friend class ItcTransportLocalTest;
	FRIEND_TEST(ItcTransportLocalTest, sendTest1);
	FRIEND_TEST(ItcTransportLocalTest, sendReceiveTest1);
	FRIEND_TEST(ItcTransportLocalTest, sendReceiveTest2);
	FRIEND_TEST(ItcTransportLocalTest, sendReceiveTest3);
	FRIEND_TEST(ItcTransportLocalTest, sendReceiveTest4);
};

using ItcMailboxRawPtr = ItcMailbox *;

} // namespace INTERNAL
} // namespace ITC