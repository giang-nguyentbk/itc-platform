#pragma once

#include <cstdint>
#include <cstddef>

#include "itcConstant.h"
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
using ItcPlatformIfReturnCode = ItcPlatformIf::ItcPlatformIfReturnCode;

/***
 * ItcAdminMessage Format:
 * [preamble] [msgno] [user payload] [endpoint]
 * 
 * + Preamble:
 *      - 4 bytes: sender           : Who sends this message.
 *      - 4 bytes: receiver         : Who receives this message.
 *      - 4 bytes: flags            : To check if message is in any mailbox's rx queue.
 *      - 4 bytes: size             : Size in bytes of [msgno] + [user payload].
 * 
 * + Message number:
 *      - 4 bytes: msgno            : Protocol which is clearly understood between senders and receivers.
 * 
 * + User payload:
 *      - unlimited bytes: payload  : Any user-defined structured data in "union ItcMessage".
 * 
 * + Endpoint:
 *      - 1 bytes: endpoint         : 0xAA, to check if message is malformed.
 * 
 * Note that: users can only see "union ItcMessage" which includes [msgno] + [user payload].
 */
struct ItcAdminMessage
{
    itc_mailbox_id_t        sender {ITC_MAILBOX_ID_DEFAULT};
    itc_mailbox_id_t        receiver {ITC_MAILBOX_ID_DEFAULT};
    uint32_t               	flags {ITC_FLAG_DEFAULT};
    uint32_t                size {ITC_MESSAGE_MSGNO_SIZE};
    
    uint32_t               	msgno {ITC_MESSAGE_MSGNO_DEFAULT};
    // uint8_t                 payload[];
    
    // uint8_t                 endpoint[1];
};

#define ITC_ADMIN_MESSAGE_ENDPOINT          (uint8_t)(0xAA)
#define ITC_ADMIN_MESSAGE_ENDPOINT_SIZE     (uint8_t)(1)
#define ITC_ADMIN_MESSAGE_PREAMBLE_SIZE     (uint32_t)(offsetof(ItcAdminMessage, msgno))
#define ITC_ADMIN_MESSAGE_MIN_SIZE          (uint32_t)(ITC_ADMIN_MESSAGE_PREAMBLE_SIZE + ITC_MESSAGE_MSGNO_SIZE + ITC_ADMIN_MESSAGE_ENDPOINT_SIZE)
#define ITC_FLAG_MESSAGE_IN_RX_QUEUE        (uint32_t)(0x1)

using ItcAdminMessageRawPtr = ItcAdminMessage *;

#define CONVERT_TO_ADMIN_MESSAGE(msg) \
    (reinterpret_cast<ItcAdminMessageRawPtr>(reinterpret_cast<uintptr_t>(msg) - ITC_ADMIN_MESSAGE_PREAMBLE_SIZE))

#define CONVERT_TO_USER_MESSAGE(admMsg) \
    (reinterpret_cast<ItcMessageRawPtr>(&admMsg->msgno))

#define GET_ITC_ADMIN_MESSAGE_ENDPOINT(admMsg) \
    (reinterpret_cast<uint8_t *>(reinterpret_cast<uintptr_t>(&adminMsg->msgno) + adminMsg->size))

class ItcAdminMessageHelper
{
public:
    static ItcAdminMessageRawPtr allocate(uint32_t msgno, size_t size = ITC_MESSAGE_MSGNO_SIZE)
    {
        if(size < ITC_MESSAGE_MSGNO_SIZE)
        {
            return nullptr;
        }

        auto adminMsg = reinterpret_cast<ItcAdminMessageRawPtr>(new uint8_t[ITC_ADMIN_MESSAGE_PREAMBLE_SIZE + size + ITC_ADMIN_MESSAGE_ENDPOINT_SIZE]);

        adminMsg->msgno = msgno;
        adminMsg->sender = ITC_MAILBOX_ID_DEFAULT;
        adminMsg->receiver = ITC_MAILBOX_ID_DEFAULT;
        adminMsg->size = size;
        adminMsg->flags = ITC_FLAG_DEFAULT;
        auto endpoint = reinterpret_cast<uint8_t *>(reinterpret_cast<uintptr_t>(&adminMsg->msgno) + size);
        *endpoint = ITC_ADMIN_MESSAGE_ENDPOINT;
        return adminMsg;
    }
    
    static bool deallocate(ItcAdminMessageRawPtr adminMsg)
    {
        if(!adminMsg)
        {
            return true;
        }

        if(adminMsg->flags & ITC_FLAG_MESSAGE_IN_RX_QUEUE)
        {
            TPT_TRACE(TRACE_ABN, "Message still in rx queue!");
            return false;
        }

        auto endpoint = GET_ITC_ADMIN_MESSAGE_ENDPOINT(adminMsg);
        if(*endpoint != ITC_ADMIN_MESSAGE_ENDPOINT)
        {
            TPT_TRACE(TRACE_ABN, SSTR("Invalid *endpoint = 0x", *endpoint & 0xFF));
            return false;
        }

        delete[] adminMsg;
        return true;
    }
};

} // namespace INTERNAL
} // namespace ITC