#include "itcTransportSysvMsgQueue.h"

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <algorithm>

#include <errno.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>

// #include <traceIf.h>
// #include "itcTptProvider.h"
#include "itcConstant.h"
#include "itcMailbox.h"
#include "itcAdminMessage.h"
#include "itcThreadManagerIf.h"
#include "itcEthernetProto.h"
#include "itcFileSystemIf.h"
#include "itcCWrapperIf.h"
#include "itcTransportLocal.h"
#include "itcPlatform.h"

using namespace ITC::INTERNAL;
using ITC::INTERNAL::ItcTransportSysvMsgQueue;

void destructRxThreadWrapper(void *args)
{
	auto inst = ItcTransportSysvMsgQueue::getInstance().lock();
	inst->destructRxThread(args);
}

void *sysvMsgQueueRxThreadWrapper(void *args)
{
	auto inst = ItcTransportSysvMsgQueue::getInstance().lock();
	return inst->sysvMsgQueueRxThread(args);
}


namespace ITC
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

using namespace ITC::PROVIDED;
using FileSystemIfReturnCode = FileSystemIf::FileSystemIfReturnCode;
using PathType = FileSystemIf::PathType;
using ItcPlatformIfReturnCode = ItcPlatformIf::ItcPlatformIfReturnCode;

SINGLETON_DEFINITION(ItcTransportSysvMsgQueue)

bool ItcTransportSysvMsgQueue::initialise(itc_mailbox_id_t regionId)
{
    if(m_isInitialised)
    {
        TPT_TRACE(TRACE_INFO, SSTR("ITC system already initialised!"));
        return false;
    }
    
    auto rc = FileSystemIf::getInstance().lock()->createPath(ITC_PATH_SYSVMQ_FILE_NAME, PathType::REGULAR_FILE, ITC_PATH_ITC_DIRECTORY_POSITION);
    if(rc != MAKE_RETURN_CODE(FileSystemIfReturnCode, ITC_FILESYSTEM_OK))
    {
        return false;
    }
    
    m_pid = CWrapperIf::getInstance().lock()->cGetPid();
    m_regionId = regionId;
    
    int32_t ret = CWrapperIf::getInstance().lock()->cPthreadKeyCreate(&m_destructKey, &destructRxThreadWrapper);
	if(ret != 0)
	{
		TPT_TRACE(TRACE_ERROR, SSTR("Failed to pthread_key_create, error code = ", ret));
		return false;
	}
	
	m_syncObj = std::make_shared<SyncObject>([](SyncObjectElementsSharedPtr elemsPtr)
	{
		auto ret = CWrapperIf::getInstance().lock()->cPthreadCondAttrSetClock(&elemsPtr->condAttrs, CLOCK_MONOTONIC);
		if(ret != 0)
		{
			TPT_TRACE(TRACE_ERROR, SSTR("Failed to pthread_condattr_setclock, error code = ", ret));
			return -1;
		}
		return 0;
	});
	m_syncObj->setTimeout(100 /* ms */);
    ThreadManagerIf::getInstance().lock()->addThread(Task(&sysvMsgQueueRxThreadWrapper), m_syncObj);
	m_isInitialised = true;
    return true;
}

void ItcTransportSysvMsgQueue::release()
{    
    if(m_isInitialised)
	{
		int ret = CWrapperIf::getInstance().lock()->cPthreadKeyDelete(m_destructKey);
		if(ret != 0)
		{
			TPT_TRACE(TRACE_ERROR, SSTR("Failed to pthread_key_delete, error code = ", ret));
			return;
		}

		m_isRxThreadTerminated = true;
		m_isInitialised = false;
	}
}

ItcPlatformIfReturnCode ItcTransportSysvMsgQueue::send(ItcAdminMessageRawPtr adminMsg)
{
    if(!m_isInitialised)
    {
        TPT_TRACE(TRACE_ERROR, SSTR("ITC Transport SYSV Message Queue not initialised yet!"));
        return MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_FAILED);
    }
    
    itc_mailbox_id_t projectId = (adminMsg->receiver & ITC_MASK_REGION_ID) >> ITC_REGION_ID_SHIFT;
	if(projectId == 0 || projectId >= ITC_MAX_SUPPORTED_REGIONS)
	{
		TPT_TRACE(TRACE_ABN, SSTR("Invalid sysv message queue peer's region id!"));
		return MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_FAILED);
	}
    
    int32_t size = ITC_ADMIN_MESSAGE_PREAMBLE_SIZE + adminMsg->size + ITC_ADMIN_MESSAGE_ENDPOINT_SIZE;
	uint8_t *txBuffer = new uint8_t[sizeof(long) + size];
    auto txMsg {reinterpret_cast<long *>(txBuffer)};
	*txMsg = (long)ITC_SYSV_MESSAGE_QUEUE_TX_MSGNO;
	auto cWrapperIf = CWrapperIf::getInstance().lock();
	cWrapperIf->cMemcpy(reinterpret_cast<void *>(txMsg + 1), adminMsg, size);
    
    while(cWrapperIf->cMsgsnd(m_contactList.at(projectId).msgQueueId, reinterpret_cast<const void *>(txMsg), size, MSG_NOERROR) == -1)
	{
		if(errno == EINTR)
		{
			continue;
		} else if(errno == EINVAL || errno == EIDRM)
		{
			TPT_TRACE(TRACE_ABN, SSTR("MSG queue of receiver has corrupted and just re-created, " \
                    "add contact list and resend msg again!"));
			removeContactInfoAtIndex(projectId);
			addContactInfoAtIndex(projectId, adminMsg->receiver);
			if(m_contactList.at(projectId).regionId == 0)
			{
				TPT_TRACE(TRACE_ERROR, SSTR("Add contact list again failed due to msq_key = -1!"));
				break;
			}
		} else
		{
			TPT_TRACE(TRACE_ERROR, SSTR("Failed to msgsnd()"));
		}
	}
	delete[] txBuffer;
	txBuffer = nullptr;
	
    /***
     * ITC System only helps to delete itc messages if the sending was successful,
     * in case of failures, users have to call ItcPlatform::delete() by themselves.
     */
    if(!ItcAdminMessageHelper::deallocate(adminMsg))
	{
		TPT_TRACE(TRACE_ERROR, SSTR("Failed to deallocate itc message!"));
		return MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_FAILED);
	}
    return MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_OK);
}

size_t ItcTransportSysvMsgQueue::getMaxMessageSize()
{
    /* struct msginfo from <bits/msg.h> included in <sys/msg.h> */
    if(m_maxMsgSize > 0)
	{
		return m_maxMsgSize;
	}
    
	struct msginfo info;
    if(CWrapperIf::getInstance().lock()->cMsgctl(0, IPC_INFO, (struct msqid_ds *)&info) == -1)
	{
		TPT_TRACE(TRACE_ABN, SSTR("Failed to msgctl()"));
		return 0;
	}
    
    int maxMsgSizeTemp = std::min(info.msgmax, info.msgmnb);
	m_maxMsgSize = (size_t)(maxMsgSizeTemp < 0 ? 0 : maxMsgSizeTemp);

	TPT_TRACE(TRACE_INFO, SSTR("Retrieve max msg size successfully, ", m_maxMsgSize, " bytes!"));
    return m_maxMsgSize;
}

void ItcTransportSysvMsgQueue::removeContactInfoAtIndex(size_t atIndex)
{
    m_contactList.at(atIndex).regionId = ITC_MAILBOX_ID_DEFAULT;
    m_contactList.at(atIndex).msgQueueId = -1;
}

void ItcTransportSysvMsgQueue::addContactInfoAtIndex(size_t atIndex, itc_mailbox_id_t mboxId)
{
	int32_t msgQueueId = getMsgQueueId(mboxId);
    if(msgQueueId != -1)
	{
		m_contactList.at(atIndex).regionId = (mboxId & ITC_MASK_REGION_ID);
		m_contactList.at(atIndex).msgQueueId = msgQueueId;
	}
}

int32_t ItcTransportSysvMsgQueue::getMsgQueueId(itc_mailbox_id_t mboxId)
{	
	itc_mailbox_id_t newMboxId = mboxId & ITC_MASK_REGION_ID;
	itc_mailbox_id_t projectId = newMboxId >> ITC_REGION_ID_SHIFT;

	key_t key = CWrapperIf::getInstance().lock()->cFtok(ITC_PATH_SYSVMQ_FILE_NAME, projectId);
	if(key == -1)
	{
		TPT_TRACE(TRACE_ERROR, SSTR("Failed to ftok"));
		return -1;
	}

	/* Use 0 to get the previously created msqid or create a new one, just to avoid unecessary EEXIST */
	int32_t msgQueueId = CWrapperIf::getInstance().lock()->cMsgGet(key, 0);
	if(msgQueueId == -1)
	{
		TPT_TRACE(TRACE_ERROR, SSTR("Failed to ftok"));
		if(errno == ENOENT)
		{
			return -1;
		}
	}
	
	return msgQueueId;
}

bool ItcTransportSysvMsgQueue::parseAndForwardMessage(uint8_t *rxBuffer, ssize_t length)
{
	auto sysvMsgno = reinterpret_cast<long *>(rxBuffer);
	if(*sysvMsgno != ITC_SYSV_MESSAGE_QUEUE_TX_MSGNO)
	{
		TPT_TRACE(TRACE_ABN, SSTR("Unknown SYSV TX MSGNO ", *sysvMsgno, " received!"));
		return false;
	}
	
	auto rxMsg = reinterpret_cast<ItcAdminMessageRawPtr>(rxBuffer + sizeof(long));
	if(length - sizeof(long) < ITC_ADMIN_MESSAGE_PREAMBLE_SIZE + rxMsg->size + ITC_ADMIN_MESSAGE_ENDPOINT_SIZE)
	{
		TPT_TRACE(TRACE_ABN, SSTR("Received malform message from some mailbox, invalid length = ", \
				length, ", rxMsg->size = ", rxMsg->size));
		return false;
	}
	
	uint8_t *endpoint = (uint8_t*)((unsigned long)(&rxMsg->msgno) + rxMsg->size);
	if(*endpoint != ITC_ADMIN_MESSAGE_ENDPOINT)
	{
		TPT_TRACE(TRACE_ABN, SSTR("Received malform message from some mailbox, invalid ENDPOINT = 0x", \
										std::hex, std::setw(2), std::setfill('0'), *endpoint & 0xFF));
		return false;
	}
	
	ItcMessageRawPtr newMsg = CONVERT_TO_USER_MESSAGE(ItcAdminMessageHelper::allocate(rxMsg->msgno, rxMsg->size));
	ItcAdminMessageRawPtr newAdminMsg = CONVERT_TO_ADMIN_MESSAGE(newMsg);
	uint32_t storedFlags = newAdminMsg->flags;
	CWrapperIf::getInstance().lock()->cMemcpy(newAdminMsg, rxMsg, ITC_ADMIN_MESSAGE_PREAMBLE_SIZE + rxMsg->size + ITC_ADMIN_MESSAGE_ENDPOINT_SIZE);
	newAdminMsg->flags = storedFlags;
	
	auto rc = ItcTransportLocal::getInstance().lock()->send(newAdminMsg);
	if(rc != MAKE_RETURN_CODE(ItcPlatformIfReturnCode, ITC_OK))
	{
		TPT_TRACE(TRACE_ABN, SSTR("Failed to forward message from sysv message queue to local transport mailbox!"));
		return false;
	}
	
	return true;
}

void ItcTransportSysvMsgQueue::destructRxThread(void *args)
{
	if(m_msgQueueId != -1)
	{
		m_isRxThreadTerminated = true;
		m_isInitialised = false;
		if(CWrapperIf::getInstance().lock()->cMsgctl(m_msgQueueId, IPC_RMID, nullptr) == -1)
		{
			TPT_TRACE(TRACE_ERROR, SSTR("Failed to msgctl"));
		}
		m_msgQueueId = -1;
	}
}

void *ItcTransportSysvMsgQueue::sysvMsgQueueRxThread(void *args)
{	
	auto cWrapperIf = CWrapperIf::getInstance().lock();
	if(!cWrapperIf)
	{
		return nullptr;
	}
	if(::prctl(PR_SET_NAME, "itcSysvMQRx", 0, 0, 0) == -1)
	{
		TPT_TRACE(TRACE_ERROR, SSTR("Failed to prctl()!"));
		return nullptr;
	}
	
	char mboxName[30];
	::sprintf(mboxName, "itc_rx_sysvmq_0x%08x", m_regionId);
	m_mboxId = ItcPlatform::getInstance().lock()->createMailbox(mboxName);
	
	TPT_TRACE(TRACE_INFO, SSTR("Starting sysvMsgQueueRxThread mailbox, ", mboxName));
	int32_t ret = cWrapperIf->cPthreadSetSpecific(m_destructKey, reinterpret_cast<void *>(m_mboxId));
	if(ret != 0)
	{
		TPT_TRACE(TRACE_ERROR, SSTR("Failed to pthread_setspecific error code = ", ret));
		return nullptr;
	}
	
	int32_t projectId = (m_regionId >> ITC_REGION_ID_SHIFT);
	key_t key = cWrapperIf->cFtok(ITC_PATH_SYSVMQ_FILE_NAME, projectId);
	if(key == -1)
	{
		TPT_TRACE(TRACE_ERROR, SSTR("Failed to ftok"));
		return nullptr;
	}
	
	m_msgQueueId = cWrapperIf->cMsgGet(key, IPC_CREAT | 0666);
	if(m_msgQueueId == -1)
	{
		TPT_TRACE(TRACE_ERROR, SSTR("Failed to msgget"));
		return nullptr;
	}

	struct msqid_ds msqinfo;
	if(cWrapperIf->cMsgctl(m_msgQueueId, IPC_STAT, &msqinfo) == -1)
	{
		TPT_TRACE(TRACE_ERROR, SSTR("Failed to msgctl"));
		return nullptr;
	}
	
	if(msqinfo.msg_qnum != 0)
	{
		// Queue should empty, if not remove it and re-create
		if(cWrapperIf->cMsgctl(m_msgQueueId, IPC_RMID, &msqinfo) == -1)
		{
			TPT_TRACE(TRACE_ERROR, SSTR("Failed to msgctl (Recreate queue)"));
			return nullptr;
		}

		m_msgQueueId = cWrapperIf->cMsgGet(key, IPC_CREAT | 0666);
		if(m_msgQueueId == -1)
		{
			TPT_TRACE(TRACE_ERROR, SSTR("Failed to msgget (Recreate queue)"));
			return nullptr;
		}
	}
	
	(void)getMaxMessageSize();
	m_rxBuffer = new uint8_t[m_maxMsgSize];
	cWrapperIf->cMemset(m_rxBuffer, 0, m_maxMsgSize);
	
	MUTEX_LOCK(&m_syncObj->elems->mtx);
	cWrapperIf->cPthreadCondSignal(&m_syncObj->elems->cond);
	MUTEX_UNLOCK(&m_syncObj->elems->mtx);

	while(true)
	{
		if(m_isRxThreadTerminated)
		{
			TPT_TRACE(TRACE_INFO, SSTR("Terminating sysvmq rx thread..."));
			break;
		}
		
		ssize_t rxLen = cWrapperIf->cMsgrcv(m_msgQueueId, m_rxBuffer, m_maxMsgSize - sizeof(long), 0, 0);
		if(rxLen < 0)
		{
			TPT_TRACE(TRACE_ERROR, SSTR("Negative rx message length, rxLen = ", rxLen));
			continue;
		}
		
		if(rxLen <= (long)(sizeof(long) + ITC_ADMIN_MESSAGE_MIN_SIZE))
		{
			TPT_TRACE(TRACE_ABN, SSTR("Received malform message from some sysv msg queue, msgSize (", rxLen, " bytes) too small!"));
			continue;
		}
		
		parseAndForwardMessage(m_rxBuffer, rxLen + sizeof(long));
	}
	
	delete[] m_rxBuffer;
	m_rxBuffer = nullptr;
	return nullptr;
}

} // namespace INTERNAL
} // namespace ITC