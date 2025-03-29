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
#include "itcThreadManager.h"


namespace ItcPlatform
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

using namespace ItcPlatform::PROVIDED;
using ItcTransportIfReturnCode = ItcTransportIf::ItcTransportIfReturnCode;
using ItcPlatformIfReturnCode = ItcPlatformIf::ItcPlatformIfReturnCode;

std::shared_ptr<ItcTransportLocal> ItcTransportLocal::m_instance = nullptr;
std::mutex ItcTransportLocal::m_singletonMutex;

std::weak_ptr<ItcTransportLocal> ItcTransportLocal::getInstance()
{
    std::scoped_lock<std::mutex> lock(m_singletonMutex);
    if (m_instance == nullptr)
    {
        m_instance.reset(new ItcTransportLocal);
    }
    return m_instance;
}

ItcTransportIfReturnCode ItcTransportLocal::initialise(itc_mailbox_id_t regionId, uint32_t mboxCount, uint32_t flags)
{
    if(m_unitInfos.size()) UNLIKELY
    {
        if(flags & ITC_FLAG_FORCE_REINIT_TRANSPORTS) LIKELY
        {
            TPT_TRACE(TRACE_INFO, SSTR("Forcibly re-initialising ITC system..."));
            for(ssize_t i = 0; i < m_unitCount; ++i)
            {
                releaseUnitInfo(i);
            }
            m_unitInfos.clear();
        } else UNLIKELY
        {
            TPT_TRACE(TRACE_INFO, SSTR("ITC system already initialised!"));
            return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_ALREADY_INITIALISED);
        }
    }
    
    m_regionId = regionId;
    m_unitCount = (0xFFFFFFFF >> COUNT_LEADING_ZEROS(mboxCount)) + 1;
    m_unitInfos = std::move(std::vector<UnitInfo>(m_unitCount));
    return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_OK);
}

ItcTransportIfReturnCode ItcTransportLocal::release()
{    
    uint32_t runningUnits {0u};
    for(const auto &unitInfo : m_unitInfos)
    {
        if(unitInfo.isInUse)
        {
            ++runningUnits;
        }
    }
    if(runningUnits > ITC_NR_INTERNAL_USED_MAILBOXES) UNLIKELY
    {
        TPT_TRACE(TRACE_ABN, SSTR("Still has remaining user-created open mailboxes, " \
            "please manually close it before releasing ITC system!"));
        return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_INVALID_OPERATION);
    }
    
    m_unitInfos.clear();
    return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_OK);
}

ItcTransportIfReturnCode ItcTransportLocal::locateItcServer(itc_mailbox_id_t *assignedRegionIdOutput, itc_mailbox_id_t *locatedItcServerMboxIdOutput)
{
    /***
     * This transport is to exchange message internally inside a Region,
     * so cannot used to locate itc-server
     */
    return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_UNDEFINED);
}

ItcTransportIfReturnCode ItcTransportLocal::createMailbox(ItcMailboxRawPtr newMbox)
{
    auto atIndex = getUnitInfoIndex(newMbox->mailboxId);
    if(atIndex < 0) UNLIKELY
    {
        return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_OUT_OF_RANGE);
    }
    
    if(m_unitInfos.at(atIndex).isInUse) UNLIKELY
    {
        TPT_TRACE(TRACE_ABN, SSTR("This mailbox ", newMbox->mailboxId, " slot already occupied by another one!"));
        return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_ALREADY_IN_USE);
    }
    
    m_unitInfos.at(atIndex).mailboxId = newMbox->mailboxId;
    m_unitInfos.at(atIndex).flags = newMbox->flags;
    m_unitInfos.at(atIndex).isInUse = true;
    return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_OK);
}

ItcTransportIfReturnCode ItcTransportLocal::deleteMailbox(ItcMailboxRawPtr mbox)
{
    auto atIndex = getUnitInfoIndex(mbox->mailboxId);
    if(atIndex < 0) UNLIKELY
    {
        return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_OUT_OF_RANGE);
    }
    
    if(!m_unitInfos.at(atIndex).isInUse) UNLIKELY
    {
        TPT_TRACE(TRACE_ABN, SSTR("Deleting a not yet created mailbox!"));
        return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_INVALID_OPERATION);
    }
    
    releaseUnitInfo(atIndex);
    return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_OK);
}

ItcTransportIfReturnCode ItcTransportLocal::sendMessage(ItcAdminMessageRawPtr adminMsg, const MailboxContactInfo &toMbox)
{
    auto atIndex = getUnitInfoIndex(toMbox.mailboxId);
    if(atIndex < 0) UNLIKELY
    {
        return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_OUT_OF_RANGE);
    }
    
    if(!m_unitInfos.at(atIndex).isInUse) UNLIKELY
    {
        TPT_TRACE(TRACE_ABN, SSTR("Sending messages to a not yet created mailbox!"));
        return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_INVALID_OPERATION);
    }
    
    m_unitInfos.at(atIndex).rxMsgQueue.push(adminMsg);
    return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_OK);
}

ItcAdminMessageRawPtr ItcTransportLocal::receiveMessage(ItcMailboxRawPtr myMbox)
{
    auto atIndex = getUnitInfoIndex(myMbox->mailboxId);
    if(atIndex < 0) UNLIKELY
    {
        return nullptr;
    }
    
    if(!m_unitInfos.at(atIndex).isInUse) UNLIKELY
    {
        TPT_TRACE(TRACE_ABN, SSTR("Receiving messages on a not yet created mailbox!"));
        return nullptr;
    }
    
    auto adminMsg = m_unitInfos.at(atIndex).rxMsgQueue.front();
    m_unitInfos.at(atIndex).rxMsgQueue.pop();
    return adminMsg;
}

size_t ItcTransportLocal::getMaxMessageSize()
{
    /***
     * No message size limitation for in-Region transport.
     */
    return 0;
}

void ItcTransportLocal::releaseUnitInfo(ssize_t atIndex)
{
    if(atIndex > (ssize_t)(m_unitInfos.size() - 1))
    {
        return;
    }
    auto &unitInfo = m_unitInfos.at(atIndex);
    auto &rxq = unitInfo.rxMsgQueue;
    while(!rxq.empty())
    {
        auto adminMsg = rxq.front();
        ItcMessageRawPtr msg = CONVERT_TO_USER_MESSAGE(adminMsg);
        if(auto rc = ItcPlatformIf::getInstance().lock()->deleteMessage(msg); rc != MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_OK)) UNLIKELY
        {
            TPT_TRACE(TRACE_ABN, SSTR("Failed to delete some message in current RX queue, atIndex = ", atIndex, ", msgno = ", adminMsg->msgno));
        }
        rxq.pop();
    }
    
    unitInfo.mailboxId = ITC_NO_MAILBOX_ID;
    unitInfo.flags = 0;
    unitInfo.isInUse = false;
}

ssize_t ItcTransportLocal::getUnitInfoIndex(uint32_t mboxId)
{ 
    if(!m_unitInfos.size()) UNLIKELY
    {
        TPT_TRACE(TRACE_ABN, SSTR("ItcTransportLocal not initialised yet!"));
        return -1;
    }
    
    if((mboxId & ITC_REGION_ID_MASK) != m_regionId) UNLIKELY
    {
        TPT_TRACE(TRACE_ABN, SSTR("This mboxId = ", mboxId, " not belong to this region/process, regionId = ", mboxId & ITC_REGION_ID_MASK));
        return -2;
    }
    
    if((mboxId & ITC_UNIT_ID_MASK) >= m_unitCount) UNLIKELY
    {
        TPT_TRACE(TRACE_ABN, SSTR("Invalid unitId = ", mboxId & ITC_UNIT_ID_MASK, ", m_unitCount = ", m_unitCount));
        return -3;
    }
    
    return mboxId & ITC_UNIT_ID_MASK;
}

} // namespace INTERNAL
} // namespace ItcPlatform