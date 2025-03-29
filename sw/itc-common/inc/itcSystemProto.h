#pragma once

#include <cstdint>
#include <thread>

#include "itc.h"

namespace ItcPlatform
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

using namespace ItcPlatform::PROVIDED;

struct itc_system_message_notify_mbox_creation_deletion_to_itc_server_request;
struct itc_system_message_notify_mbox_creation_deletion_to_itc_server_reply;
struct itc_system_message_locate_mbox_in_itc_server_request;
struct itc_system_message_locate_mbox_in_itc_server_reply;
using ItcPlatform::PROVIDED::itc_system_message_locate_mbox_in_itc_server_result; /* Definition in itc.h header file */

union ItcMessage {
    uint32_t								                        				msgno;
    struct itc_system_message_notify_mbox_creation_deletion_to_itc_server_request	m_itc_system_message_notify_mbox_creation_deletion_to_itc_server_request;
    struct itc_system_message_notify_mbox_creation_deletion_to_itc_server_reply		m_itc_system_message_notify_mbox_creation_deletion_to_itc_server_reply;
    struct itc_system_message_locate_mbox_in_itc_server_request						m_itc_system_message_locate_mbox_in_itc_server_request;
    struct itc_system_message_locate_mbox_in_itc_server_reply		    			m_itc_system_message_locate_mbox_in_itc_server_reply;
    struct itc_system_message_locate_mbox_in_itc_server_result						m_itc_system_message_locate_mbox_in_itc_server_result;
    // struct itc_fwd_data_to_itcgws			itc_fwd_data_to_itcgws;
    // struct itc_get_namespace_request		itc_get_namespace_request;
    // struct itc_get_namespace_reply			itc_get_namespace_reply;
};

static constexpr uint32_t ITC_SYSTEM_MESSAGE_NOTIFY_MBOX_CREATION_DELETION_TO_ITC_SERVER_REQUEST		= (ITC_SYSTEM_MESSAGE_NUMBER_BASE + 0x1);
static constexpr uint32_t ITC_SYSTEM_MESSAGE_NOTIFY_MBOX_CREATION_DELETION_TO_ITC_SERVER_REPLY			= (ITC_SYSTEM_MESSAGE_NUMBER_BASE + 0x2);
static constexpr uint32_t ITC_SYSTEM_MESSAGE_LOCATE_MBOX_IN_ITC_SERVER_REQUEST 							= (ITC_SYSTEM_MESSAGE_NUMBER_BASE + 0x3);
static constexpr uint32_t ITC_SYSTEM_MESSAGE_LOCATE_MBOX_IN_ITC_SERVER_REPLY							= (ITC_SYSTEM_MESSAGE_NUMBER_BASE + 0x4);
// static constexpr uint32_t ITC_SYSTEM_MESSAGE_LOCATE_MBOX_IN_ITC_SERVER_RESULT						= (ITC_SYSTEM_MESSAGE_NUMBER_BASE + 0x5); // Defined in itc-api header files such as itc.h

static constexpr uint32_t ITC_MESSAGE_REPLY_RETURN_CODE_UNDEFINED 	= 0x1;
static constexpr uint32_t ITC_MESSAGE_REPLY_RETURN_CODE_CONFIRMED 	= 0x2;
static constexpr uint32_t ITC_MESSAGE_REPLY_RETURN_CODE_REJECTED 	= 0x3;

struct itc_system_message_notify_mbox_creation_deletion_to_itc_server_request {
	uint32_t    		msgno {ITC_MESSAGE_NUMBER_UNDEFINED};
	uint32_t			isCreation {0}; /* Otherwise it's isDeletion */
	itc_mailbox_id_t    mboxId {ITC_NO_MAILBOX_ID};
	char	    		mboxName[1];
};

struct itc_system_message_notify_mbox_creation_deletion_to_itc_server_reply {
	uint32_t    		msgno {ITC_MESSAGE_NUMBER_UNDEFINED};
	itc_mailbox_id_t    processedMboxId {ITC_NO_MAILBOX_ID};
	uint32_t			returnCode {ITC_MESSAGE_REPLY_RETURN_CODE_UNDEFINED};
};

struct itc_system_message_locate_mbox_in_itc_server_request {
	uint32_t    		msgno {ITC_MESSAGE_NUMBER_UNDEFINED};
	uint32_t			isSync {0}; /* Otherwise it's isAsync */
	itc_mailbox_id_t    fromMboxId {ITC_NO_MAILBOX_ID};
	uint32_t			timeout {0};
	uint32_t			mode {ITC_MODE_LOCATE_IN_ALL};
	char				locatedMboxName[1];
};

struct itc_system_message_locate_mbox_in_itc_server_reply {
	uint32_t			msgno {ITC_MESSAGE_NUMBER_UNDEFINED};
	uint32_t			returnCode {ITC_MESSAGE_REPLY_RETURN_CODE_UNDEFINED};
	char				locatedMboxName[1];
};


} // namespace INTERNAL
} // namespace ItcPlatform