#pragma once

#include <cstdint>
#include <thread>

#include "itc.h"
#include "itcAdminMessage.h"

namespace ITC
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

using namespace ITC::PROVIDED;

#define ITC_SYSTEM_MESSAGE_NOTIFY_MBOX_CREATION_DELETION_TO_ITC_SERVER_REQUEST		(uint32_t)(ITC_SYSTEM_MESSAGE_NUMBER_BASE + 0x1)
#define ITC_SYSTEM_MESSAGE_NOTIFY_MBOX_CREATION_DELETION_TO_ITC_SERVER_REPLY		(uint32_t)(ITC_SYSTEM_MESSAGE_NUMBER_BASE + 0x2)
#define ITC_SYSTEM_MESSAGE_LOCATE_MBOX_SYNC_IN_ITC_SERVER_REQUEST 					(uint32_t)(ITC_SYSTEM_MESSAGE_NUMBER_BASE + 0x3)
#define ITC_SYSTEM_MESSAGE_LOCATE_MBOX_ASYNC_IN_ITC_SERVER_REQUEST 					(uint32_t)(ITC_SYSTEM_MESSAGE_NUMBER_BASE + 0x4)
// #define ITC_SYSTEM_MESSAGE_LOCATE_MBOX_IN_ITC_SERVER_REPLY							(uint32_t)(ITC_SYSTEM_MESSAGE_NUMBER_BASE + 0x5) // Defined in itc-api header files such as itc.h
#define ITC_SYSTEM_MESSAGE_FORWARD_MESSAGE_TO_ITC_SERVER_REQUEST					(uint32_t)(ITC_SYSTEM_MESSAGE_NUMBER_BASE + 0x6)


struct itc_system_message_notify_mbox_creation_deletion_to_itc_server_request
{
	uint32_t    		msgno {ITC_MESSAGE_MSGNO_DEFAULT};
	uint32_t			isCreation {1}; /* 2 means it's isDeletion */
	itc_mailbox_id_t    mboxId {ITC_MAILBOX_ID_DEFAULT};
	uint32_t			isExternalCommunicationNeeded {0};
	char	    		mboxName[1];
};

struct itc_system_message_notify_mbox_creation_deletion_to_itc_server_reply
{
	uint32_t    		msgno {ITC_MESSAGE_MSGNO_DEFAULT};
	itc_mailbox_id_t    processedMboxId {ITC_MAILBOX_ID_DEFAULT};
};

struct itc_system_message_locate_mbox_sync_in_itc_server_request
{
	uint32_t    		msgno {ITC_MESSAGE_MSGNO_DEFAULT};
	uint32_t			mode {ITC_MODE_LOCATE_IN_ALL};
	char				locatedMboxName[1];
};

struct itc_system_message_locate_mbox_async_in_itc_server_request
{
	uint32_t    		msgno {ITC_MESSAGE_MSGNO_DEFAULT};
	uint32_t			mode {ITC_MODE_LOCATE_IN_ALL};
	
	/***
	 * Already found in local mailbox list. In this case, itc-server will take the mailboxId,
	 * and on behalf send back to calling mailbox instead of directly getting from return value
	 * of locateMailboxAsync function call.
	 */
	uint32_t			replyImmediately {0};
	uint32_t			mailboxId {ITC_MAILBOX_ID_DEFAULT};
	
	char				locatedMboxName[1];
};

struct itc_system_message_forward_message_to_itc_server_request
{
	uint32_t			msgno {ITC_MESSAGE_MSGNO_DEFAULT};
	itc_mailbox_id_t	toWorldId {ITC_MAILBOX_ID_DEFAULT};
	uint32_t			flattenMsgLength {ITC_ADMIN_MESSAGE_MIN_SIZE};
	uint8_t				flattenMsg[1];
};


} // namespace INTERNAL
} // namespace ITC

using namespace ITC::INTERNAL;
union ItcMessage
{
    uint32_t								                        				msgno;
    struct itc_system_message_notify_mbox_creation_deletion_to_itc_server_request	m_itc_system_message_notify_mbox_creation_deletion_to_itc_server_request;
    struct itc_system_message_notify_mbox_creation_deletion_to_itc_server_reply		m_itc_system_message_notify_mbox_creation_deletion_to_itc_server_reply;
    struct itc_system_message_locate_mbox_sync_in_itc_server_request				m_itc_system_message_locate_mbox_sync_in_itc_server_request;
	struct itc_system_message_locate_mbox_async_in_itc_server_request				m_itc_system_message_locate_mbox_async_in_itc_server_request;
    struct itc_system_message_locate_mbox_in_itc_server_reply		    			m_itc_system_message_locate_mbox_in_itc_server_reply;
	struct itc_system_message_forward_message_to_itc_server_request					m_itc_system_message_forward_message_to_itc_server_request;
};