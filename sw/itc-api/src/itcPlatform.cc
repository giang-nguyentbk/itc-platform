#include "itcPlatform.h"

#include <cstdint>
#include <string>
#include <unistd.h>

#include "itcFileSystemIf.h"
#include "itcConstant.h"
#include "itcTransportLocal.h"
#include "itcTransportLSocket.h"
#include "itcTransportSysvMsgQueue.h"
#include "itcSystemProto.h"

using namespace ITC::INTERNAL;
using ITC::INTERNAL::ItcPlatform;

void destructMailboxAtThreadExitWrapper(void *args)
{
	auto inst = ItcPlatform::getInstance().lock();
	inst->destructMailboxAtThreadExit(args);
}

namespace ITC
{
namespace PROVIDED
{
/***
 * Just for compilation in unit testing.
 * Those getInstance from <InterfaceClassName>If interface classes
 * must look like this to avoid multiple definition compiler error
 * which is conflicts between this real getInstance implementation
 * and the one is in mock version.
 * 
 * For example, find itcFileSystemIfMock.cc to see mocked implementation
 * of getInstance().
*/
#ifndef UNITTEST
SINGLETON_IF_DEFINITION(ItcPlatformIf, ItcPlatform)
#endif
}
    
namespace INTERNAL
{

// using namespace CommonUtils::V1::EnumUtils;
using namespace ITC::PROVIDED;
using ItcPlatformIfReturnCode = ItcPlatformIf::ItcPlatformIfReturnCode;
using ThreadManagerIfReturnCode = ThreadManagerIf::ThreadManagerIfReturnCode;

SINGLETON_DEFINITION(ItcPlatform)

thread_local ItcMailboxRawPtr ItcPlatform::m_myMailbox = nullptr;

ItcPlatform::ItcPlatform()
{
    static SmartContainer<ItcMailbox>::Callback callback =
    {
        [](const ItcMailbox &mbox) -> std::string
        {
            return mbox.name;
        },
        [](ItcMailbox &mbox)
        {
            mbox.reset();
        }
    };
    m_mboxList = std::make_shared<SmartContainer<ItcMailbox>>(callback);
}

ItcPlatform::~ItcPlatform()
{
	if(m_mboxList)
    {
        m_mboxList.reset();
    }
}

ItcPlatformIfReturnCode ItcPlatform::initialise(uint32_t flags)
{
    if(!checkAndStartItcServer())
    {
        return MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_FAILED);
    }
    
    if(m_isInitialised)
    {
        return MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_OK);
    }
    
    if(flags & ITC_FLAG_I_AM_ITC_SERVER)
    {
        m_regionId = 1 << ITC_REGION_ID_SHIFT;
        m_itcServerMboxId = m_regionId | 1;
    } else
    {
        auto locatedResults = ItcTransportLSocket::getInstance().lock()->locateItcServer();
        if(locatedResults.assignedRegionId == ITC_MAILBOX_ID_DEFAULT || locatedResults.itcServerMboxId == ITC_MAILBOX_ID_DEFAULT)
        {
            return MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_FAILED);
        }
        
        m_regionId = locatedResults.assignedRegionId;
        m_itcServerMboxId = locatedResults.itcServerMboxId;
    }
    
    bool areTransportsInitialised {true};
    areTransportsInitialised &= ItcTransportLocal::getInstance().lock()->initialise(m_mboxList);
    areTransportsInitialised &= ItcTransportLSocket::getInstance().lock()->initialise(m_regionId);
    areTransportsInitialised &= ItcTransportSysvMsgQueue::getInstance().lock()->initialise(m_regionId);
    if(!areTransportsInitialised)
    {
        return MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_FAILED);
    }
    
    auto ret = CWrapperIf::getInstance().lock()->cPthreadKeyCreate(&m_destructKey, destructMailboxAtThreadExitWrapper);
	if(ret != 0)
	{
		TPT_TRACE(TRACE_ERROR, SSTR("Failed to create destruct_key, error code = ", ret));
		return MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_FAILED);
	}
    
    auto rc = ThreadManagerIf::getInstance().lock()->startAllThreads();
    if(rc != MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_OK))
    {
        return MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_FAILED);
    }
    
    return MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_OK);
}

ItcPlatformIfReturnCode ItcPlatform::release()
{
    if(!m_isInitialised)
    {
        return MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_OK);
    }
    
    auto numberOfActiveMboxes = m_mboxList->size();
    if(numberOfActiveMboxes > ITC_NR_INTERNAL_USED_MAILBOXES)
    {
        return MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_FAILED);
    }
    
    auto rc = ThreadManagerIf::getInstance().lock()->terminateAllThreads();
    if(rc != MAKE_RETURN_CODE(ThreadManagerIfReturnCode, THREAD_MANAGER_OK))
    {
        return MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_FAILED);
    }
    
    m_mboxList->clear();
    
    ItcTransportLSocket::getInstance().lock()->release();
    ItcTransportSysvMsgQueue::getInstance().lock()->release();
    
    auto ret = CWrapperIf::getInstance().lock()->cPthreadKeyDelete(m_destructKey);
	if(ret != 0)
	{
		// ERROR trace is needed here
		TPT_TRACE(TRACE_ERROR, SSTR("pthread_key_delete error code = ", ret));
		return MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_FAILED);
	}
    
    return MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_OK);
}

ItcMessageRawPtr ItcPlatform::allocateMessage(uint32_t msgno, size_t size)
{
    auto adminMsg = ItcAdminMessageHelper::allocate(msgno, size);
    return CONVERT_TO_USER_MESSAGE(adminMsg);
}

ItcPlatformIfReturnCode ItcPlatform::deallocateMessage(ItcMessageRawPtr msg)
{
    if(ItcAdminMessageHelper::deallocate(CONVERT_TO_ADMIN_MESSAGE(msg)))
    {
        return MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_OK);
    }
    return MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_FAILED);
}

itc_mailbox_id_t ItcPlatform::createMailbox(const std::string &name, uint32_t flags)
{
    if(!m_isInitialised)
    {
        return ITC_MAILBOX_ID_DEFAULT;
    }
    
    if(m_myMailbox)
    {
        return m_myMailbox->mailboxId;
    }
    
    if(auto indexOpt = m_mboxList->emplace(ItcMailbox(name, flags)); indexOpt.has_value())
    {
        if(auto mboxOpt = m_mboxList->at(indexOpt.value()); mboxOpt.has_value())
        {
            mboxOpt.value().get().mailboxId = m_regionId | (indexOpt.value() & ITC_MASK_UNIT_ID);
        }
        m_myMailbox = m_mboxList->data(indexOpt.value());
    } else
    {
        return ITC_MAILBOX_ID_DEFAULT;
    }
    
    auto cWrapperIf = CWrapperIf::getInstance().lock();
    auto ret = cWrapperIf->cPthreadSetSpecific(m_destructKey, m_myMailbox);
	if(ret != 0)
	{
		// ERROR trace is needed here
		TPT_TRACE(TRACE_ERROR, SSTR("Failed to pthread_setspecific, error code = ", ret));
		return ITC_MAILBOX_ID_DEFAULT;
	}
    
    if(m_regionId != (m_itcServerMboxId | ITC_MASK_REGION_ID))
    {
        auto req = allocateMessage(ITC_SYSTEM_MESSAGE_NOTIFY_MBOX_CREATION_DELETION_TO_ITC_SERVER_REQUEST, offsetof(itc_system_message_notify_mbox_creation_deletion_to_itc_server_request, mboxName) + name.length() + 1);
        req->m_itc_system_message_notify_mbox_creation_deletion_to_itc_server_request.isCreation = 1;
        req->m_itc_system_message_notify_mbox_creation_deletion_to_itc_server_request.mboxId = m_myMailbox->mailboxId;
        req->m_itc_system_message_notify_mbox_creation_deletion_to_itc_server_request.isExternalCommunicationNeeded = (flags & ITC_FLAG_EXTERNAL_COMMUNICATION_NEEDED) ? 1 : 0;
        cWrapperIf->cStrcpy(req->m_itc_system_message_notify_mbox_creation_deletion_to_itc_server_request.mboxName, name.c_str()); 
        if(send(req, MailboxContactInfo(m_itcServerMboxId)) != MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_OK))
        {
            deallocateMessage(req);
            return ITC_MAILBOX_ID_DEFAULT;
        }
    }
    
    return m_myMailbox->mailboxId;
}

ItcPlatformIfReturnCode ItcPlatform::deleteMailbox(itc_mailbox_id_t mboxId)
{
    if(!m_isInitialised)
    {
        return MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_FAILED);
    }
    
    if(mboxId != m_myMailbox->mailboxId)
    {
        TPT_TRACE(TRACE_ERROR, SSTR("Not allowed to delete other thread's mailbox, mbox_id = 0x", std::hex, std::setw(2), std::setfill('0'), mboxId));
        return MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_FAILED);
    }
    
    std::string mboxName = m_myMailbox->name;
    
    size_t index = mboxId & ITC_MASK_UNIT_ID;
    m_mboxList->remove(index);
    
    auto cWrapperIf = CWrapperIf::getInstance().lock();
    if(m_regionId != (m_itcServerMboxId | ITC_MASK_REGION_ID))
    {
        auto req = allocateMessage(ITC_SYSTEM_MESSAGE_NOTIFY_MBOX_CREATION_DELETION_TO_ITC_SERVER_REQUEST, offsetof(itc_system_message_notify_mbox_creation_deletion_to_itc_server_request, mboxName) + mboxName.length() + 1);
        req->m_itc_system_message_notify_mbox_creation_deletion_to_itc_server_request.isCreation = 2;
        req->m_itc_system_message_notify_mbox_creation_deletion_to_itc_server_request.mboxId = m_myMailbox->mailboxId;
        cWrapperIf->cStrcpy(req->m_itc_system_message_notify_mbox_creation_deletion_to_itc_server_request.mboxName, mboxName.c_str()); 
        if(send(req, MailboxContactInfo(m_itcServerMboxId)) != MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_OK))
        {
            deallocateMessage(req);
            return MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_FAILED);
        }
    }
    
    m_myMailbox = nullptr;
    return MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_OK);
}

ItcPlatformIfReturnCode ItcPlatform::send(ItcMessageRawPtr msg, const MailboxContactInfo &toMbox)
{
    if(!m_isInitialised)
    {
        return MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_FAILED);
    }
    
    if(!m_myMailbox)
    {
        return MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_FAILED);
    }
    
    if(toMbox.mailboxId == m_myMailbox->mailboxId && toMbox.worldId == 0)
    {
        return MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_FAILED);
    }
    
    auto adminMsg = CONVERT_TO_ADMIN_MESSAGE(msg);
    adminMsg->receiver = toMbox.mailboxId;
    adminMsg->sender = m_myMailbox->mailboxId;
    
    if(toMbox.worldId != 0)
    {
        return forwardMessageToItcServer(adminMsg, toMbox.worldId);
    } else
    {
        if((toMbox.mailboxId & ITC_MASK_REGION_ID) != m_regionId)
        {
            return ItcTransportSysvMsgQueue::getInstance().lock()->send(adminMsg);
        } else
        {
            return ItcTransportLocal::getInstance().lock()->send(adminMsg);
        }
    }
    
    return MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_FAILED);
}

ItcMessageRawPtr ItcPlatform::receive(uint32_t mode, uint32_t timeout)
{
    if(!m_isInitialised)
    {
        return nullptr;
    }
    
    if(!m_myMailbox)
    {
        return nullptr;
    }
    
    auto adminMsg = ItcTransportLocal::getInstance().lock()->receive(m_myMailbox, mode, timeout);
    return CONVERT_TO_USER_MESSAGE(adminMsg);
}

MailboxContactInfo ItcPlatform::locateMailboxSync(const std::string &mboxName, uint32_t mode, uint32_t timeout)
{
    MailboxContactInfo info;
    if(!m_isInitialised || !m_myMailbox)
    {
        return info;
    }
    
    uint32_t locateMode = mode & ITC_MASK_LOCATE;
    if(locateMode & ITC_MODE_LOCATE_IN_REGION)
    {
        if(auto mboxOpt = m_mboxList->at(mboxName); mboxOpt.has_value())
        {
            info.mailboxId = mboxOpt.value().get().mailboxId;
            return info;
        }
    }
    
    if(!(locateMode & ITC_MODE_LOCATE_IN_WORLD) && !(locateMode & ITC_MODE_LOCATE_IN_UNIVERSE))
    {
        return info;
    }
    
    /* If cannot find in local, send locating-mailbox requests to itc-server. */
    auto req = allocateMessage(ITC_SYSTEM_MESSAGE_LOCATE_MBOX_SYNC_IN_ITC_SERVER_REQUEST, offsetof(itc_system_message_locate_mbox_sync_in_itc_server_request, locatedMboxName) + mboxName.length() + 1);
    req->m_itc_system_message_locate_mbox_sync_in_itc_server_request.mode = locateMode;
    CWrapperIf::getInstance().lock()->cStrcpy(req->m_itc_system_message_locate_mbox_sync_in_itc_server_request.locatedMboxName, mboxName.c_str());
    
    auto rc = send(req, MailboxContactInfo(m_itcServerMboxId));
    deallocateMessage(req);
    if(rc != MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_OK))
    {
        return info;
    }
    
    uint32_t timeoutMode = mode & ITC_MASK_TIMEOUT;
    auto rep = receive(timeoutMode, timeout);
    if(!rep)
    {
        return info;
    }
        
    if(rep->m_itc_system_message_locate_mbox_in_itc_server_reply.msgno != ITC_SYSTEM_MESSAGE_LOCATE_MBOX_IN_ITC_SERVER_REPLY)
    {
        return info;
    }
    
    info.mailboxId = rep->m_itc_system_message_locate_mbox_in_itc_server_reply.locatedMbox.mailboxId;
    info.worldId = rep->m_itc_system_message_locate_mbox_in_itc_server_reply.locatedMbox.worldId;
    return info;
}

ItcPlatformIfReturnCode ItcPlatform::locateMailboxAsync(const std::string &mboxName, uint32_t mode)
{
    if(!m_isInitialised || !m_myMailbox)
    {
        return MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_FAILED);
    }

    uint32_t locateMode = mode & ITC_MASK_LOCATE;
    auto req = allocateMessage(ITC_SYSTEM_MESSAGE_LOCATE_MBOX_ASYNC_IN_ITC_SERVER_REQUEST, offsetof(itc_system_message_locate_mbox_async_in_itc_server_request, locatedMboxName) + mboxName.length() + 1);
    req->m_itc_system_message_locate_mbox_async_in_itc_server_request.mode = locateMode;
    CWrapperIf::getInstance().lock()->cStrcpy(req->m_itc_system_message_locate_mbox_async_in_itc_server_request.locatedMboxName, mboxName.c_str());
    
    if(auto mboxOpt = m_mboxList->at(mboxName); mboxOpt.has_value())
    {
        req->m_itc_system_message_locate_mbox_async_in_itc_server_request.replyImmediately = 1;
        req->m_itc_system_message_locate_mbox_async_in_itc_server_request.mailboxId = mboxOpt.value().get().mailboxId;
    }
    
    auto rc = send(req, MailboxContactInfo(m_itcServerMboxId));
    deallocateMessage(req);
    return rc;
}

itc_mailbox_id_t ItcPlatform::getSender(const ItcMessageRawPtr &msg)
{
    const auto adminMsg = CONVERT_TO_ADMIN_MESSAGE(msg);
    return adminMsg->sender;
}

itc_mailbox_id_t ItcPlatform::getReceiver(const ItcMessageRawPtr &msg)
{
    const auto adminMsg = CONVERT_TO_ADMIN_MESSAGE(msg);
    return adminMsg->receiver;
}

size_t ItcPlatform::getMsgSize(const ItcMessageRawPtr &msg)
{
    const auto adminMsg = CONVERT_TO_ADMIN_MESSAGE(msg);
    return adminMsg->size;
}

int32_t ItcPlatform::myMailboxFd()
{
    if(m_myMailbox)
    {
        return m_myMailbox->getMboxFd();
    }
    return -1;
}

std::string ItcPlatform::getMailboxName(itc_mailbox_id_t mboxId)
{
    if(m_myMailbox)
    {
        return m_myMailbox->name;
    }
    return "";
}

bool ItcPlatform::startDaemon(const std::string &programPath)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        std::cout << "[ERROR] Failed to fork process" << std::endl;
        return false;
    } else if (pid > 0)
    {
        // Parent process exits, leaving the daemon running
        std::cout << "[INFO] Daemon started with PID: " << pid << std::endl;
        return true;
    }

    // Child process becomes the daemon
    if (setsid() < 0)
    {
        std::cout << "[ERROR] Failed to create new session" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Second fork to prevent reacquiring a terminal
    pid = fork();
    if (pid < 0)
    {
        std::cout << "[ERROR] Second fork failed" << std::endl;
    } else if (pid > 0)
    {
        exit(EXIT_SUCCESS); // Parent exits, child continues
    }

    // Change working directory to root
    if(chdir("/") < 0)
    {
        std::cout << "[ERROR] Failed to chdir, errno = " << errno << std::endl;
    }

    // Execute the target daemon program
    char* args[] = { (char*)programPath.c_str(), nullptr };
    execvp(args[0], args);

    // If execvp fails
    std::cout << "[ERROR] Failed to execvp daemon, errno = " << errno << "!\n";
    exit(EXIT_FAILURE);
}

bool ItcPlatform::checkAndStartItcServer()
{
    if(!FileSystemIf::getInstance().lock()->exists(ITC_PATH_ITC_SERVER_SOCKET))
    {
        return startDaemon(ITC_PATH_ITC_SERVER_PROGRAM);
    }
    return false;
}

ItcPlatformIfReturnCode ItcPlatform::forwardMessageToItcServer(ItcAdminMessageRawPtr adminMsg, itc_mailbox_id_t toWorldId)
{
    uint32_t flattenMsgLength = ITC_ADMIN_MESSAGE_PREAMBLE_SIZE + adminMsg->size + ITC_ADMIN_MESSAGE_ENDPOINT_SIZE;
    auto req = allocateMessage(ITC_SYSTEM_MESSAGE_FORWARD_MESSAGE_TO_ITC_SERVER_REQUEST, offsetof(itc_system_message_forward_message_to_itc_server_request, flattenMsg) + flattenMsgLength);
    
    req->m_itc_system_message_forward_message_to_itc_server_request.toWorldId = toWorldId;
    req->m_itc_system_message_forward_message_to_itc_server_request.flattenMsgLength = flattenMsgLength;
    CWrapperIf::getInstance().lock()->cMemcpy(req->m_itc_system_message_forward_message_to_itc_server_request.flattenMsg, adminMsg, flattenMsgLength);
    
    auto rc = send(req, MailboxContactInfo(m_itcServerMboxId));
    ItcAdminMessageHelper::deallocate(adminMsg);
    return rc;
}

void ItcPlatform::destructMailboxAtThreadExit(void *args)
{
    auto myMbox = reinterpret_cast<ItcMailboxRawPtr>(args);
    if(m_mboxList->exist(myMbox->name))
    {
        deleteMailbox(myMbox->mailboxId);
    }
}

} // namespace INTERNAL
} // namespace ITC
