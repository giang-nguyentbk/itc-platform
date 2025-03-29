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

static constexpr uint32_t ITC_ETHERNET_MESSAGE_NUMBER_BASE 		    			= (ITC_SYSTEM_BASE + 0x100);
static constexpr uint32_t ITC_ETHERNET_MESSAGE_LOCATE_ITC_SERVER_REQUEST 		= (ITC_ETHERNET_MESSAGE_NUMBER_BASE + 0x1);
static constexpr uint32_t ITC_ETHERNET_MESSAGE_LOCATE_ITC_SERVER_REPLY			= (ITC_ETHERNET_MESSAGE_NUMBER_BASE + 0x2);

struct itc_ethernet_message_locate_itc_server_request {
	uint32_t    		msgno {ITC_MESSAGE_NUMBER_UNDEFINED};
	int32_t    			pid {-1};
};

struct itc_ethernet_message_locate_itc_server_reply {
	uint32_t    		msgno {ITC_MESSAGE_NUMBER_UNDEFINED};
	itc_mailbox_id_t    assignedRegionId {ITC_NO_MAILBOX_ID};
	itc_mailbox_id_t	itcServerMboxId {ITC_NO_MAILBOX_ID};
};


} // namespace INTERNAL
} // namespace ItcPlatform