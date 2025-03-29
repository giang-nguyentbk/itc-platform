#pragma once

#include <cstdint>
#include <cstddef>

#include "itcConstant.h"
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

struct ItcAdminMessage {
/* ItcAdminMessage is only used in itc infrastructure ifself, through below preamble information,
users should not use ItcAdminMessage but use ItcMessage instead */
/*** PREAMBLE - USER HIDDEN */
    /* DO NOT change anything in the remainder - this is a core part - to avoid breaking the whole ITC system. */
    itc_mailbox_id_t        sender {ITC_NO_MAILBOX_ID};
    itc_mailbox_id_t        receiver {ITC_NO_MAILBOX_ID};
    uint32_t               	flags {ITC_FLAG_NO_FLAG};
    uint32_t                size {sizeof(uint32_t)};   // Size of the ItcMessage that user gives to allocate ItcAdminMessage
/*** PREAMBLE - USER HIDDEN */

/*** ITC MESSAGE - USER VISIBILITY */
    /* This is the ItcMessage part users can see and use it */
    uint32_t               	msgno {ITC_MESSAGE_NUMBER_UNDEFINED};
    /* Starting of user data, optional */
    /* ...                             */

    /* There is one byte called endpoint which should be always 0xAA,
    if not, then there is something wrong with your itc message */
    /* Why it's 0xAA, because 0xAA = 1010 1010. Most efficient way to confirm the itc message correctness */
    // char                     payload_startpoint[1]; // Actual user data will start from this byte address

	/* Payload in bytes
	*  ....
	*  Payload in bytes */
/*** ITC MESSAGE - USER VISIBILITY */

	/* char			endpoint; // Will be 0xAA */
    
    ItcAdminMessage(itc_mailbox_id_t iSender = ITC_NO_MAILBOX_ID, itc_mailbox_id_t iReceiver = ITC_NO_MAILBOX_ID,
        uint32_t iFlags = ITC_FLAG_NO_FLAG, uint32_t iSize = sizeof(uint32_t), uint32_t iMsgno = ITC_MESSAGE_NUMBER_UNDEFINED)
        : sender(iSender),
          receiver(iReceiver),
          flags(iFlags),
          size(iSize),
          msgno(iMsgno)
    {}
    ~ItcAdminMessage() = default; 
};

static constexpr uint32_t ITC_ADMIN_MESSAGE_PREAMBLE_SIZE   = offsetof(ItcAdminMessage, msgno);
static constexpr uint32_t ITC_ADMIN_MESSAGE_MIN_SIZE   = ITC_ADMIN_MESSAGE_PREAMBLE_SIZE + sizeof(uint32_t) + 1;
static constexpr uint8_t ITC_ADMIN_MESSAGE_ENDPOINT         = 0xAA;
using ItcAdminMessageRawPtr = ItcAdminMessage *;

} // namespace INTERNAL
} // namespace ItcPlatform