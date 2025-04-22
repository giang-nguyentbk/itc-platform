#pragma once

#include <cstdint>
#include <string>
#include <memory>

// #include <enumUtils.h>

// using namespace CommonUtils::V1::EnumUtils;
/***
 * TODO: move this macro into enumUtils.h in common-utils project
 */
// #define MAKE_RETURN_CODE(EnumWrapperClass, enumRawValue) "\"
// 	EnumWrapperClass(EnumWrapperClass##Raw::enumRawValue)

/***
 * TODO: For compilation only (TBD)
 */
#define MAKE_RETURN_CODE(EnumClassWrapper, rawValue) \
	EnumClassWrapper::rawValue

/***
 * TODO: For compilation only (TBD)
 */
#define TPT_TRACE(a, b)

using ItcMessageRawPtr = union ItcMessage *;

namespace ITC
{
namespace PROVIDED
{
#define ITC_MAX_SUPPORTED_MAILBOXES 							(uint32_t)(1024)
#define ITC_MAILBOX_ID_DEFAULT 									(uint32_t)(0xFFFFFFFF)
#define ITC_MESSAGE_MSGNO_DEFAULT 								(uint32_t)(0xFFFFFFFF)
#define ITC_MESSAGE_MSGNO_SIZE 									(uint32_t)(sizeof(uint32_t))
#define ITC_FLAG_DEFAULT 										(uint32_t)(0b0)
#define ITC_FLAG_EXTERNAL_COMMUNICATION_NEEDED 					(uint32_t)(0b1)
#define ITC_MODE_DEFAULT 										(uint32_t)(0b0)
#define ITC_MODE_RECEIVE_NON_BLOCKING							(uint32_t)(0b1)
#define ITC_MODE_LOCATE_IN_REGION								(uint32_t)(0b10)
#define ITC_MODE_LOCATE_IN_WORLD								(uint32_t)(0b100)
#define ITC_MODE_LOCATE_IN_UNIVERSE								(uint32_t)(0b1000)
#define ITC_MODE_LOCATE_IN_ALL									(uint32_t)(0b1110)
#define ITC_MASK_LOCATE											(uint32_t)(0x1110)
#define ITC_SYSTEM_BASE 										(uint32_t)(0x00000000)
#define ITC_SYSTEM_MESSAGE_NUMBER_BASE 							(uint32_t)(ITC_SYSTEM_BASE + 0x10)
#define ITC_SYSTEM_MESSAGE_LOCATE_MBOX_IN_ITC_SERVER_REPLY		(uint32_t)(ITC_SYSTEM_MESSAGE_NUMBER_BASE + 0x5)

using itc_mailbox_id_t = uint32_t;

struct MailboxContactInfo
{
	itc_mailbox_id_t mailboxId {ITC_MAILBOX_ID_DEFAULT};
	uint32_t worldId {0};
	MailboxContactInfo(itc_mailbox_id_t iMailboxId = ITC_MAILBOX_ID_DEFAULT, uint32_t iWorldId = 0)
		: mailboxId(iMailboxId),
		  worldId(iWorldId)
	{}
	~MailboxContactInfo() = default;
};

struct itc_system_message_locate_mbox_in_itc_server_reply {
	uint32_t			msgno {ITC_MESSAGE_MSGNO_DEFAULT}; // Must be ITC_SYSTEM_MESSAGE_LOCATE_MBOX_IN_ITC_SERVER_REPLY
	MailboxContactInfo 	locatedMbox;
};

class ItcPlatformIf
{
public:
    static std::weak_ptr<ItcPlatformIf> getInstance();
	virtual ~ItcPlatformIf() = default;
    
    ItcPlatformIf(const ItcPlatformIf& other) = delete;
    ItcPlatformIf& operator=(const ItcPlatformIf& other) = delete;
    ItcPlatformIf(ItcPlatformIf&& other) noexcept = delete;
    ItcPlatformIf& operator=(ItcPlatformIf&& other) noexcept = delete;

	/***
	 * TODO: For compilation only (TBD)
	 */
	enum class ItcPlatformIfReturnCode
	{
		ITC_UNDEFINED,
		ITC_OK,
		ITC_FAILED,
	};

	// enum class ItcPlatformIfReturnCodeRaw
	// {
	// 	ITC_UNDEFINED           =   0b0,                /* 0		- Default undefined return code */
	// 	ITC_OK 				    = 	0b1,			    /* 1		- Everything is good */
	// 	ITC_ALREADY_USED  		= 	0b10,			    /* 2		- The mailbox id already used by someone */
	// 	ITC_ALREADY_INITIALISED = 	0b100,			    /* 4		- Already calling local_init() */
	// 	ITC_NOT_INITIALISED  	=	0b1000,			    /* 8		- Not calling local_init() yet */
	// 	ITC_QUEUE_NULL 			=	0b10000,			/* 16		- Not calling local_create_mailbox yet */
	// 	ITC_QUEUE_EMPTY			=	0b100000,		    /* 32		- This is not really a problem at all */
	// 	ITC_NOT_THIS_PROC		=	0b1000000,		    /* 64		- Three highest hexes of mailbox id != my_mbox_id_in_itccoord */
	// 	ITC_OUT_OF_RANGE		=	0b10000000,		    /* 128		- Local_mb_id > nr_localmbx_datas */
	// 	ITC_NOT_DEL_ALL_MBOX	=	0b100000000,		/* 256		- Not deleting all user mailboxes before itc_exit() */
	// 	ITC_DEL_IN_WRONG_STATE	=	0b1000000000,		/* 512		- Delete a mailbox when it's not created yet */
	// 	ITC_FREE_NULL_PTR		=	0b10000000000,		/* 1024		- Attempts to remove null qitem */
	// 	ITC_INVALID_ARGUMENTS	=	0b100000000000,		/* 2048		- Validation of function parameters is invalid */
	// 	ITC_SYSCALL_ERROR		=	0b1000000000000,    /* 4096		- System call return error: pthread, sysv message queue,... */
	// 	ITC_INVALID_MSG_SIZE	=	0b10000000000000,	/* 8192		- Message length received too large or short */
	// 	ITC_INVALID_RESPONSE	=	0b100000000000000	/* 16384 	- Received an invalid response */
	// };

	// class ItcPlatformIfReturnCode : public EnumType<ItcPlatformIfReturnCodeRaw>
	// {
	// public:
	// 	explicit ItcPlatformIfReturnCode(const ItcPlatformIfReturnCodeRaw& raw) : EnumType<ItcPlatformIfReturnCodeRaw>(raw) {}
	// 	explicit ItcPlatformIfReturnCode() : EnumType<ItcPlatformIfReturnCodeRaw>(ItcPlatformIfReturnCodeRaw::ITC_UNDEFINED) {}

	// 	std::string toString() const override
	// 	{
	// 		switch (getRawEnum())
	// 		{
	// 		case ItcPlatformIfReturnCodeRaw::ITC_UNDEFINED:
	// 			return "ITC_UNDEFINED";

	// 		case ItcPlatformIfReturnCodeRaw::ITC_OK:
	// 			return "ITC_OK";

	// 		case ItcPlatformIfReturnCodeRaw::ITC_ALREADY_USED:
	// 			return "ITC_ALREADY_USED";

	// 		case ItcPlatformIfReturnCodeRaw::ITC_ALREADY_INITIALISED:
	// 			return "ITC_ALREADY_INITIALISED";
			
	// 		case ItcPlatformIfReturnCodeRaw::ITC_NOT_INITIALISED:
	// 			return "ITC_NOT_INITIALISED";

	// 		default:
	// 			return "Unknown EnumType: " + std::to_string(toS32());
	// 		}
	// 	}
	// };

    /***
     * ITC API declarations
     */
	/***
	 * Currently, flags is only used internally in ITC system to identify if the process that has called initialise()
	 * is itc-server or not, external usage is reserved.
	 * + ITC_FLAG_I_AM_ITC_SERVER	0b1
	 */
    virtual ItcPlatformIfReturnCode initialise(uint32_t flags = ITC_FLAG_DEFAULT) = 0;
	
	/***
	 * Users must ensure the deletion of all user-created mailboxes before calling this ITC system's release.
	 */
	virtual ItcPlatformIfReturnCode release() = 0;
	
	virtual ItcMessageRawPtr allocateMessage(uint32_t msgno, size_t size = ITC_MESSAGE_MSGNO_SIZE) = 0;
	virtual ItcPlatformIfReturnCode deallocateMessage(ItcMessageRawPtr msg) = 0;
	
	/***
	 * Currently "flags" (OR bits) indicate whether:
	 * 		+ ITC_FLAG_EXTERNAL_COMMUNICATION_NEEDED = 0b1: The created mailbox desires to have external communication to other Worlds or not.
	 */
	virtual itc_mailbox_id_t createMailbox(const std::string &name, uint32_t flags = ITC_FLAG_DEFAULT) = 0;
	virtual ItcPlatformIfReturnCode deleteMailbox(itc_mailbox_id_t mboxId) = 0;
	
	/***
	 * In case of sending messages inside a Region/World only, leave worldId the default initialised value.
	 * Otherwise you may need to provide worldId in case of outside World communication required.
	 * To have worldId, you may need to locate mailbox first via either locateMailboxSync or locateMailboxAsync.
	 */
	virtual ItcPlatformIfReturnCode send(ItcMessageRawPtr msg, const MailboxContactInfo &toMbox) = 0;
	
	/***
	 * There are 1 modes:
	 * + ITC_MODE_RECEIVE_NON_BLOCKING
	 */
	virtual ItcMessageRawPtr receive(uint32_t mode = ITC_MODE_DEFAULT) = 0;
	
	/***
	 * To locate mailboxes, you must give itc-server a mode (OR bits)
	 * mode:
	 * 		+ ITC_MODE_LOCATE_IN_REGION: 		0b00100
	 * 		+ ITC_MODE_LOCATE_IN_WORLD: 		0b01000
	 * 		+ ITC_MODE_LOCATE_IN_UNIVERSE: 		0b10000
	 * 		+ ITC_MODE_LOCATE_IN_ALL			0b11100
	 * 
	 * For example, if you desire to locate a mailbox with name "mailboxABC",
	 * and you already know that this mailbox may be in the same Region or World as yours,
	 * you need to give itc-server a mode = ITC_MODE_LOCATE_IN_REGION | ITC_MODE_LOCATE_IN_WORLD,
	 * then itc-server will only search in the Region our mailbox belongs to and our World,
	 * no search outside is done for performance optimization.
	 * If you're not sure where you should look for, let's just use ITC_MODE_LOCATE_IN_ALL.
	 * 
	 * Please note that locating mailboxes in Universe may be time consuming
	 * than searching internally inside a World or Region.
	 * 
	 * mode = (ITC_MODE_TIMEOUT_* | ITC_MODE_LOCATE_*)
	 */
	virtual MailboxContactInfo locateMailboxSync(const std::string &mboxName, uint32_t mode = ITC_MODE_LOCATE_IN_ALL, uint32_t timeout = 0) = 0;
	
	/***
	 * Instead of blocking on locating requests and waiting for results synchronously from itc-server side,
	 * you can just add "struct itc_system_message_locate_mbox_in_itc_server_reply" into your own "union ItcMessage {...}",
	 * implement a proper handler for msgno ITC_SYSTEM_MESSAGE_LOCATE_MBOX_IN_ITC_SERVER_REPLY and use locateMailboxAsync().
	 * Once locateMailboxAsync() is called, it will send a locating request to itc-server, immediately reply without waiting for reply message and let itc-server continue locating the requested mailbox itself.
	 * 
	 * When the result is available, either timeout, not found or the target mailbox is located,
	 * itc-server will populate an ItcMessage ITC_SYSTEM_MESSAGE_LOCATE_MBOX_IN_ITC_SERVER_REPLY
	 * with corresponding information, and send back to our mailbox, who was just requesting for locating another.
	 * 
	 * mode = ITC_MODE_LOCATE_*
	 */
	virtual ItcPlatformIfReturnCode locateMailboxAsync(const std::string &mboxName, uint32_t mode = ITC_MODE_LOCATE_IN_ALL) = 0;
	
	/***
	 * Helper functions
	 */
	virtual itc_mailbox_id_t getSender(const ItcMessageRawPtr &msg) = 0;
	virtual itc_mailbox_id_t getReceiver(const ItcMessageRawPtr &msg) = 0;
	virtual size_t getMsgSize(const ItcMessageRawPtr &msg) = 0;
	virtual int32_t myMailboxFd() = 0;
	virtual std::string getMailboxName(itc_mailbox_id_t mboxId) = 0;

protected:
    ItcPlatformIf() = default;
}; // class ItcPlatformIf

} // namespace PROVIDED

namespace REQUIRED
{
	
} // namespace REQUIRED
} // namespace ITC
