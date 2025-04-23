#include "itcTransportLocal.h"

#include <cstdint>
#include <cstddef>
#include <mutex>

// #include <traceIf.h>
// #include "itcTptProvider.h"

#include "itc.h"
#include "itcConstant.h"
#include "itcMailbox.h"
#include "itcAdminMessage.h"


namespace ITC
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{
SINGLETON_DEFINITION(ItcTransportLocal)

using namespace ITC::PROVIDED;

bool ItcTransportLocal::initialise(std::shared_ptr<ConcurrentContainer<ItcMailbox, ITC_MAX_SUPPORTED_MAILBOXES>> mboxList)
{    
    m_mboxList = mboxList;
    return true;
}

ItcPlatformIfReturnCode ItcTransportLocal::send(ItcAdminMessageRawPtr adminMsg)
{
    size_t receiverIndex = adminMsg->receiver & ITC_MASK_UNIT_ID;
    auto receiver = m_mboxList.lock()->at(receiverIndex);
    if(receiver)
    {
        receiver->push(adminMsg);
        return MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_OK);
    }
    return MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_FAILED);
}

ItcAdminMessageRawPtr ItcTransportLocal::receive(ItcMailboxRawPtr myMbox, uint32_t mode)
{
    return myMbox->pop(mode);
}

} // namespace INTERNAL
} // namespace ITC