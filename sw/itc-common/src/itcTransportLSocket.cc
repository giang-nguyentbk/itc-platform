#include "itcTransportLSocket.h"

#include <cstdint>
#include <cstddef>
#include <mutex>

#include <errno.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

// #include <traceIf.h>
// #include "itcTptProvider.h"
#include "itcConstant.h"
#include "itcMailbox.h"
#include "itcAdminMessage.h"
#include "itcThreadManager.h"
#include "itcEthernetProto.h"
#include "itcFileSystemIf.h"
#include "itcCWrapperIf.h"


namespace ItcPlatform
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

using ItcTransportIfReturnCode = ItcTransportIf::ItcTransportIfReturnCode;
using FileSystemIfReturnCode = FileSystemIf::FileSystemIfReturnCode;
using PathType = FileSystemIf::PathType;

std::shared_ptr<ItcTransportLSocket> ItcTransportLSocket::m_instance = nullptr;
std::mutex ItcTransportLSocket::m_singletonMutex;

std::weak_ptr<ItcTransportLSocket> ItcTransportLSocket::getInstance()
{
    std::scoped_lock<std::mutex> lock(m_singletonMutex);
    if (m_instance == nullptr)
    {
        m_instance.reset(new ItcTransportLSocket);
    }
    return m_instance;
}

ItcTransportIfReturnCode ItcTransportLSocket::initialise(itc_mailbox_id_t regionId, uint32_t mboxCount, uint32_t flags)
{
    if(m_isItcServerRunning) LIKELY
    {
        auto cWrapperIf = CWrapperIf::getInstance().lock();
        if(!cWrapperIf)
        {
            return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_SYSCALL_ERROR);
        }
        if(!m_isLSockPathCreated)
        {
            auto rc = FileSystemIf::getInstance().lock()->createPath(ITC_PATH_SOCK_FOLDER_NAME, std::filesystem::perms::all, 2, PathType::DIRECTORY);
            if(rc != MAKE_RETURN_CODE(FileSystemIfReturnCode, ITC_FILESYSTEM_OK))
            {
                return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_SYSCALL_ERROR);
            }
            m_isLSockPathCreated = true;
        }
        
        auto sockFd = cWrapperIf->cSocket(AF_LOCAL, SOCK_STREAM, 0);
        if(sockFd < 0) UNLIKELY
        {
            TPT_TRACE(TRACE_ERROR, SSTR("Failed to open LSocket!"));
            return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_SYSCALL_ERROR);
        }
        
        sockaddr_un itcServerAddr;
        cWrapperIf->cMemset(&itcServerAddr, 0, sizeof(sockaddr_un));
        itcServerAddr.sun_family = AF_LOCAL;
        ::sprintf(itcServerAddr.sun_path, "%s_0x%08x", ITC_PATH_LSOCK_BASE_FILE_NAME.c_str(), regionId);
        
        /***
         * When using connect() remember always use sizeof(coord_addr) here. If you use sizeof(sockaddr) instead,
         * there will be a bug that "No such file or directory" even though it's existing, Idk why :)
         */
        int32_t res = cWrapperIf->cConnect(sockFd, (const sockaddr*)&itcServerAddr, sizeof(itcServerAddr));
        if(res < 0) UNLIKELY
        {
            TPT_TRACE(TRACE_ERROR, SSTR("Failed to connect to address ", itcServerAddr.sun_path, ", res = ", res, ", errno = ", errno));
            return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_SYSCALL_ERROR);
        }
        
        char ack[4] = {0}; /* Only enough space for string "ack\0" */
        ssize_t rxLen {0};
        do
        {
            /***
             * When this socket file descriptor is created, itc-server will scan it and try to connect to it,
             * and then send back to us a string "ack\0"
             */
            rxLen = cWrapperIf->cRecv(sockFd, ack, 4, 0);
        } while (rxLen < 0 && errno == EINTR);
        
        if(rxLen < 0) UNLIKELY
        {
            TPT_TRACE(TRACE_ABN, SSTR("ACK from itc-server is not received, rxLen = ", rxLen));
            return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_SYSCALL_ERROR);
        } else if(rxLen != 4 || cWrapperIf->cStrcmp("ack", ack) != 0) UNLIKELY
        {
            return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_UNEXPECTED_RESPONSE);
        }
        
        m_sockFd = sockFd;
        return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_OK);
    }
    return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_ITC_SERVER_NOT_RUNNING);
}

ItcTransportIfReturnCode ItcTransportLSocket::release()
{    
    if(m_isItcServerRunning)
    {
        auto ret = CWrapperIf::getInstance().lock()->cClose(m_sockFd);
        if(ret < 0) UNLIKELY
        {
            TPT_TRACE(TRACE_ERROR, SSTR("Failed to close LSocket!"));
            return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_SYSCALL_ERROR);
        }
        
        auto rc = FileSystemIf::getInstance().lock()->removePath(ITC_PATH_SOCK_FOLDER_NAME);
        if(rc != MAKE_RETURN_CODE(FileSystemIfReturnCode, ITC_FILESYSTEM_OK))
        {
            TPT_TRACE(TRACE_ERROR, SSTR("Failed to remove socket path ", ITC_PATH_SOCK_FOLDER_NAME, "!"));
            return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_SYSCALL_ERROR);
        }
        
        m_sockFd = -1;
        m_isItcServerRunning = false;
        m_isLSockPathCreated = false;
    }
    return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_OK);
}

ItcTransportIfReturnCode ItcTransportLSocket::locateItcServer(itc_mailbox_id_t *assignedRegionId, itc_mailbox_id_t *locatedItcServerMboxId)
{
    auto cWrapperIf = CWrapperIf::getInstance().lock();
    if(!cWrapperIf)
    {
        return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_SYSCALL_ERROR);
    }
    
    int32_t sockFd = cWrapperIf->cSocket(AF_LOCAL, SOCK_STREAM, 0);
	if(sockFd < 0) UNLIKELY
	{
		TPT_TRACE(TRACE_ERROR, "Failed to call socket()!");
		return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_SYSCALL_ERROR);
	}
    
    struct sockaddr_un itcServerAddr;
    cWrapperIf->cMemset(&itcServerAddr, 0, sizeof(struct sockaddr_un));
    itcServerAddr.sun_family = AF_LOCAL;
	cWrapperIf->cStrcpy(itcServerAddr.sun_path, ITC_PATH_ITC_SERVER_FILE_NAME.c_str());
    
    int32_t ret = cWrapperIf->cConnect(sockFd, (struct sockaddr*)&itcServerAddr, sizeof(itcServerAddr));
	if(ret < 0) UNLIKELY
	{
		TPT_TRACE(TRACE_ERROR, SSTR("Failed to connect(), errno = ", errno));
		return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_SYSCALL_ERROR);
	}

    itc_ethernet_message_locate_itc_server_request lrequest;
	lrequest.msgno  = ITC_ETHERNET_MESSAGE_LOCATE_ITC_SERVER_REQUEST;
	lrequest.pid    = cWrapperIf->cGetPid();
    
    ret = cWrapperIf->cSend(sockFd, &lrequest, sizeof(itc_ethernet_message_locate_itc_server_request), 0);
    if(ret < 0) UNLIKELY
	{
		TPT_TRACE(TRACE_ERROR, SSTR("Failed to send ITC_ETHERNET_MESSAGE_LOCATE_ITC_SERVER_REQUEST!"));
		return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_SYSCALL_ERROR);
	}
    
    uint8_t rxBuffer[ITC_MAX_SOCKET_RX_BUFFER_SIZE];
    auto lreply = reinterpret_cast<itc_ethernet_message_locate_itc_server_reply *>(rxBuffer);
    auto receivedBytes = cWrapperIf->cRecv(sockFd, lreply, ITC_MAX_SOCKET_RX_BUFFER_SIZE, 0);
	if(receivedBytes < (ssize_t)sizeof(itc_ethernet_message_locate_itc_server_reply)) UNLIKELY
	{
		TPT_TRACE(TRACE_ABN, SSTR("Invalid ITC_ETHERNET_MESSAGE_LOCATE_ITC_SERVER_REQUEST received, receivedBytes = ", receivedBytes));
		return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_INVALID_RESPONSE);
	}
    
    cWrapperIf->cClose(sockFd);
    
    /* Done communication, start analyzing the response */
	if(lreply->msgno == ITC_ETHERNET_MESSAGE_LOCATE_ITC_SERVER_REPLY && lreply->assignedRegionId != ITC_NO_MAILBOX_ID) LIKELY
	{
        if(assignedRegionId) LIKELY
        {
            *assignedRegionId = lreply->assignedRegionId;
        }
        if(locatedItcServerMboxId) LIKELY
        {
            *locatedItcServerMboxId = lreply->itcServerMboxId;
        }
        m_isItcServerRunning = true;
	} else UNLIKELY
	{
		/* Indicate that we have received a strange message type in response */
		TPT_TRACE(TRACE_ABN, SSTR("Unexpected ITC_ETHERNET_MESSAGE_LOCATE_ITC_SERVER_REPLY information received!"));
		return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_INVALID_RESPONSE);
	}
    
    return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_OK);
}

ItcTransportIfReturnCode ItcTransportLSocket::createMailbox(ItcMailboxRawPtr newMbox)
{
    /***
     * NOT USED
     */
    return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_UNDEFINED);
}

ItcTransportIfReturnCode ItcTransportLSocket::deleteMailbox(ItcMailboxRawPtr mbox)
{
    /***
     * NOT USED
     */
    return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_UNDEFINED);
}

ItcTransportIfReturnCode ItcTransportLSocket::sendMessage(ItcAdminMessageRawPtr adminMsg, const MailboxContactInfo &toMbox)
{
    /***
     * NOT USED
     */
    return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_UNDEFINED);
}

ItcAdminMessageRawPtr ItcTransportLSocket::receiveMessage(ItcMailboxRawPtr myMbox)
{
    /***
     * NOT USED
     */
    return nullptr;
}

size_t ItcTransportLSocket::getMaxMessageSize()
{
    /***
     * NOT USED
     */
    return 0;
}

/***
 * @DEPRECATED
 */
DEPRECATED("Please use new alternative functions in FileSystemIf instead!")
ItcTransportIfReturnCode ItcTransportLSocket::createLSockPath()
{
    auto ret = CWrapperIf::getInstance().lock()->cMkdir(ITC_PATH_SOCK_FOLDER_NAME.c_str(), 0777);
    if(ret < 0 && errno != EEXIST) UNLIKELY
	{
		TPT_TRACE(TRACE_ERROR, SSTR("Failed to mkdir() ", ITC_PATH_SOCK_FOLDER_NAME));
		return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_SYSCALL_ERROR);
	} else if(ret < 0 && errno == EEXIST)
	{
		TPT_TRACE(TRACE_INFO, SSTR("LSock Path ", ITC_PATH_SOCK_FOLDER_NAME, " already exists!"));
	}
    
    ret = CWrapperIf::getInstance().lock()->cChmod(ITC_PATH_SOCK_FOLDER_NAME.c_str(), 0777);
	if(ret < 0) UNLIKELY
	{
		TPT_TRACE(TRACE_ERROR, SSTR("Failed to chmod ", ITC_PATH_SOCK_FOLDER_NAME, ", errno = ", errno));
		return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_SYSCALL_ERROR);
	}
    
    m_isLSockPathCreated = true;
    return MAKE_RETURN_CODE(ItcTransportIfReturnCode, ITC_TRANSPORT_OK);
}

} // namespace INTERNAL
} // namespace ItcPlatform