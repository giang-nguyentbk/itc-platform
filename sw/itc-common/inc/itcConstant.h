#pragma once

#include <cstdint>

#include "itcAdminMessage.h"
#include "itc.h"

namespace ItcPlatform
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

static constexpr uint32_t ITC_FLAG_FORCE_REINIT_TRANSPORTS          = 0x00000001;
static constexpr uint32_t ITC_FLAG_I_AM_ITC_SERVER                  = 0x00000001;
static constexpr uint32_t ITC_UNIT_ID_MASK                          = 0x0000FFFF;
static constexpr uint32_t ITC_REGION_ID_MASK                        = 0xFFF00000;
static constexpr uint32_t ITC_REGION_ID_SHIFT                       = 20; /* Right shift by 20 bits to get region ID */
static constexpr uint32_t ITC_MAX_SOCKET_RX_BUFFER_SIZE             = 1024;

#define COUNT_LEADING_ZEROS(val)    __builtin_clz(val)

#define ITC_NR_INTERNAL_USED_MAILBOXES 1

#define CONVERT_TO_ADMIN_MESSAGE(msg) \
    reinterpret_cast<ItcAdminMessageRawPtr>(reinterpret_cast<uintptr_t>(msg) - ITC_ADMIN_MESSAGE_PREAMBLE_SIZE)

#define CONVERT_TO_USER_MESSAGE(admMsg) \
    reinterpret_cast<ItcMessageRawPtr>(&admMsg->msgno)

} // namespace INTERNAL
} // namespace ItcPlatform