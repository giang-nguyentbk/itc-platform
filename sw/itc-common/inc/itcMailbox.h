#pragma once

#include <cstdint>
#include <memory>
#include <string>

// #include <enumUtils.h>

#include "itc.h"
#include "itcConstant.h"

namespace ItcPlatform
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

using namespace ItcPlatform::PROVIDED;

enum class MailboxState
{
    MBOX_STATE_UNDEFINED,
	MBOX_STATE_UNUSED,
	MBOX_STATE_INUSE,
	MBOX_NR_OF_STATES
};

// enum class MailboxStateRaw
// {
//     MBOX_STATE_UNDEFINED = -1,
// 	MBOX_STATE_UNUSED,
// 	MBOX_STATE_INUSE,
// 	MBOX_NR_OF_STATES
// };

// class MailboxState : public EnumType<MailboxStateRaw>
// {
// public:
// 	explicit MailboxState(const MailboxStateRaw& raw) : EnumType<MailboxStateRaw>(raw) {}
// 	explicit MailboxState() : EnumType<MailboxStateRaw>(MailboxStateRaw::MBOX_STATE_UNDEFINED) {}

// 	std::string toString() const override
// 	{
// 		switch (getRawEnum())
// 		{
//         case MailboxStateRaw::MBOX_STATE_UNDEFINED:
// 			return "MBOX_STATE_UNDEFINED";

// 		case MailboxStateRaw::MBOX_STATE_UNUSED:
// 			return "MBOX_STATE_UNUSED";

// 		case MailboxStateRaw::MBOX_STATE_INUSE:
// 			return "MBOX_STATE_INUSE";

// 		default:
// 			return "Unknown MailboxState EnumType: " + std::to_string(toS32());
// 		}
// 	}
// };

struct RxQueueInfo {
	pthread_mutex_t			mutex;
    pthread_mutexattr_t		mutexAttr;
	pthread_cond_t			condVar;
	int32_t				    fd;
	bool				    isFdCreated;
	uint32_t			    messageCount;
	bool				    isInRx;
};

struct ItcMailbox {
    std::string     	name;
    itc_mailbox_id_t   	mailboxId {ITC_NO_MAILBOX_ID};
    MailboxState		state {MailboxState::MBOX_STATE_UNDEFINED};
    uint32_t        	flags {ITC_FLAG_NO_FLAG};
    RxQueueInfo	    	rxqInfo;
	
	ItcMailbox(std::string &&iName, itc_mailbox_id_t iMboxId = ITC_NO_MAILBOX_ID, MailboxState iState = MailboxState::MBOX_STATE_UNDEFINED, uint32_t iFlags = ITC_FLAG_NO_FLAG)
		: name(iName),
		  mailboxId(iMboxId),
		  state(iState),
		  flags(iFlags)
	{}
	~ItcMailbox() = default;
};

using ItcMailboxRawPtr = ItcMailbox *;

} // namespace INTERNAL
} // namespace ItcPlatform