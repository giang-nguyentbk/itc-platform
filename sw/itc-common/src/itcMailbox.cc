#include "itcMailbox.h"

namespace ITC
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

void ItcMailbox::enqueueAndNotify(ItcAdminMessageRawPtr msg)
{
    MUTEX_LOCK(&m_syncObj->elems->mtx);
    // std::cout << "ETRUGIA: thread = " << std::this_thread::get_id() << ", enqueueAndNotify ENTER, cond = " << std::hex << &m_syncObj->elems->cond << std::endl;
    push(msg);
    notify();
    MUTEX_UNLOCK(&m_syncObj->elems->mtx);
    // std::cout << "ETRUGIA: thread = " << std::this_thread::get_id() << ", enqueueAndNotify EXIT, cond = " << std::hex << &m_syncObj->elems->cond << std::endl;
}

ItcAdminMessageRawPtr ItcMailbox::dequeue(uint32_t mode, uint32_t timeout)
{
    if(mode == ITC_MODE_TIMEOUT_WAIT_FOR_MSEC)
    {
        m_syncObj->setTimeout(timeout);
    }
    
    auto cWrapperIf = CWrapperIf::getInstance().lock();
    ItcAdminMessageRawPtr adminMsg {nullptr};
    MUTEX_LOCK(&m_syncObj->elems->mtx);
    do
    {
        flags |= ITC_FLAG_MAILBOX_IN_RX;
        adminMsg = pop();
        if(!adminMsg)
        {
            if(mode == ITC_MODE_TIMEOUT_NO_WAIT)
            {
                flags &= ~ITC_FLAG_MAILBOX_IN_RX;
                break;
            } else if(mode == ITC_MODE_TIMEOUT_WAIT_FOREVER)
            {
                // std::cout << "ETRUGIA: thread = " << std::this_thread::get_id() << ", Waiting on pthread_cond_wait, cond = " << std::hex << &m_syncObj->elems->cond << std::endl;
                auto ret = cWrapperIf->cPthreadCondWait(&m_syncObj->elems->cond, &m_syncObj->elems->mtx);
                // std::cout << "ETRUGIA: thread = " << std::this_thread::get_id() << ", Someone has trigger pthread_cond_wait\n";
                if(ret != 0)
                {
                    break;
                }
            } else
            {
                m_syncObj->calculateExpiredDate();
                auto ret = cWrapperIf->cPthreadCondTimedWait(&m_syncObj->elems->cond, &m_syncObj->elems->mtx, &m_syncObj->timeout);
                if(ret == ETIMEDOUT)
                {
                    // TRACE TIMEOUT
                    break;
                } else if(ret != 0)
                {
                    // TRACE ERROR
                    break;
                }
            }
        } else
        {
            m_syncObj->elems->fdEventCounter--;
            /* Only call cRead to reset the eventFd's kernel counter
            once receiver mailbox handles all messages from rx queue. */
            if(m_syncObj->elems->fd != -1 && m_syncObj->elems->fdEventCounter == 0)
            {
                uint8_t buffer[sizeof(uint64_t)];
                if(cWrapperIf->cRead(m_syncObj->elems->fd, &buffer, sizeof(uint64_t)) < 0)
                {
                    ItcAdminMessageHelper::deallocate(adminMsg);
                    adminMsg = nullptr;
                    break;
                }
            }
        }
        
        flags &= ~ITC_FLAG_MAILBOX_IN_RX;
    } while(!adminMsg);
    MUTEX_UNLOCK(&m_syncObj->elems->mtx);
    return adminMsg;
}

int32_t ItcMailbox::getMboxFd()
{
    return m_syncObj->elems->fd;
}

void ItcMailbox::notify()
{
    /***
     * System call write() below will create a cancellation point
     * that can cause this thread get cancelled unexpectedly.
     * So, use PTHREAD_CANCEL_DISABLE here to temporarily disable canceling ability.
     */
    int32_t oldCancelState;
    auto cWrapperIf = CWrapperIf::getInstance().lock();
    cWrapperIf->cPthreadSetCancelState(PTHREAD_CANCEL_DISABLE, &oldCancelState);

    /* Trigger eventFd counter once via cWrite, on subsequent itc message sends
    of a batch we only maintain fdEventCounter via post increment operation. */
    if(m_syncObj->elems->fd != -1 && m_syncObj->elems->fdEventCounter == 0)
    {
        uint64_t one = 1;
        if(cWrapperIf->cWrite(m_syncObj->elems->fd, &one, sizeof(uint64_t)) < 0)
        {
            TPT_TRACE(TRACE_ERROR, "Failed to write()!");
        }
    }
    
    m_syncObj->elems->fdEventCounter++;
    // std::cout << "ETRUGIA: thread = " << std::this_thread::get_id() << ", Before pthread_cond_signal, cond = " << std::hex << &m_syncObj->elems->cond << std::endl;
    cWrapperIf->cPthreadCondSignal(&m_syncObj->elems->cond);
    // std::cout << "ETRUGIA: thread = " << std::this_thread::get_id() << ", After pthread_cond_signal, cond = " << std::hex << &m_syncObj->elems->cond << std::endl;
    cWrapperIf->cPthreadSetCancelState(oldCancelState, NULL);
}

void ItcMailbox::push(ItcAdminMessageRawPtr msg)
{
    m_rxMsgQueue.push(msg);
}

ItcAdminMessageRawPtr ItcMailbox::pop()
{
    // std::cout << "ETRUGIA: thread = " << std::this_thread::get_id() << ", pop ENTER\n";
    ItcAdminMessageRawPtr msg {nullptr};
    if(!m_rxMsgQueue.empty())
    {
        msg = m_rxMsgQueue.front();
        m_rxMsgQueue.pop();
    }
    // std::cout << "ETRUGIA: thread = " << std::this_thread::get_id() << ", pop EXIT, msg = " << std::hex << msg << std::endl;
    return msg;
}

void ItcMailbox::reset()
{
    name = "";
    mailboxId = ITC_MAILBOX_ID_DEFAULT;
    flags = ITC_FLAG_DEFAULT;
    
    if(!m_syncObj)
    {
        return;
    }
    
    MUTEX_LOCK(&m_syncObj->elems->mtx);
    while(!m_rxMsgQueue.empty())
    {
        ItcAdminMessageHelper::deallocate(m_rxMsgQueue.front());
        m_rxMsgQueue.pop();
    }
    MUTEX_UNLOCK(&m_syncObj->elems->mtx);
    
    m_syncObj->reset();
    m_syncObj.reset();
}

} // namespace INTERNAL
} // namespace ITC