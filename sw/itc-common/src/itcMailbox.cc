#include "itcMailbox.h"

namespace ITC
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

bool ItcMailbox::push(ItcAdminMessageRawPtr msg)
{
    bool active = m_isActive.load(MEMORY_ORDER_ACQUIRE);
    if(!active) UNLIKELY
    {
        return false;
    }
    m_rxMsgQueue->push(msg);
    return true;
    // return m_rxMsgQueue->tryPush(msg);
}

// push
// test4 average: 369.50 ns
// test3 average: 426.02 ns
// test2 average: 512.58 ns
// test1 average: 81.68 ns

// tryPush
// test4 average: 303.52 ns
// test3 average: 387.56 ns
// test2 average: 447.24 ns
// test1 average: 75.92 ns



ItcAdminMessageRawPtr ItcMailbox::pop(uint32_t mode)
{
    bool active = m_isActive.load(MEMORY_ORDER_ACQUIRE);
    if(!active) UNLIKELY
    {
        return nullptr;
    }
    if(mode & ITC_MODE_RECEIVE_NON_BLOCKING)
    {
        ItcAdminMessageRawPtr msg {nullptr};
        m_rxMsgQueue->tryPop(msg);
        return msg;
    }
    return m_rxMsgQueue->pop();
}

void ItcMailbox::setState(bool newState)
{
    bool expected = !newState;
    if(m_isActive.compare_exchange_strong(expected, newState, MEMORY_ORDER_RELEASE, MEMORY_ORDER_ACQUIRE))
    {
        if(!newState)
        {
            m_mailboxId = ITC_MAILBOX_ID_DEFAULT;
            m_flags = ITC_FLAG_DEFAULT;
            while(!m_rxMsgQueue->empty())
            {
                ItcAdminMessageRawPtr adminMsg {nullptr};
                auto success = m_rxMsgQueue->tryPop(adminMsg);
                if(success && adminMsg)
                {
                    ItcAdminMessageHelper::deallocate(adminMsg);
                }
            }
        }
    }
}

} // namespace INTERNAL
} // namespace ITC