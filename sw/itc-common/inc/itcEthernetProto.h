#pragma once

#include <cstdint>
#include <thread>

#include "itc.h"

namespace ITC
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

using namespace ITC::PROVIDED;

#define ITC_ETHERNET_MESSAGE_NUMBER_BASE 		    			(uint32_t)(ITC_SYSTEM_BASE + 0x100)
#define ITC_ETHERNET_MESSAGE_LOCATE_ITC_SERVER_REQUEST 			(uint32_t)(ITC_ETHERNET_MESSAGE_NUMBER_BASE + 0x1)
#define ITC_ETHERNET_MESSAGE_LOCATE_ITC_SERVER_REPLY			(uint32_t)(ITC_ETHERNET_MESSAGE_NUMBER_BASE + 0x2)

struct itc_ethernet_message_locate_itc_server_request
{
	uint32_t    		msgno {ITC_MESSAGE_MSGNO_DEFAULT};
	int32_t    			pid {-1};
};

struct itc_ethernet_message_locate_itc_server_reply
{
	uint32_t    		msgno {ITC_MESSAGE_MSGNO_DEFAULT};
	itc_mailbox_id_t    assignedRegionId {ITC_MAILBOX_ID_DEFAULT};
	itc_mailbox_id_t	itcServerMboxId {ITC_MAILBOX_ID_DEFAULT};
};


} // namespace INTERNAL
} // namespace ITC