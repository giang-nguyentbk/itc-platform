#include "itcTransportLSocket.h"

#include <cstdint>
#include <cstddef>

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
#include "itcEthernetProto.h"
#include "itcFileSystemIf.h"
#include "itcCWrapperIf.h"


namespace ITC
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{
using FileSystemIfReturnCode = FileSystemIf::FileSystemIfReturnCode;
using PathType = FileSystemIf::PathType;

SINGLETON_DEFINITION(ItcTransportLSocket)

bool ItcTransportLSocket::initialise(itc_mailbox_id_t regionId)
{
    if(m_isItcServerRunning)
    {
        auto cWrapperIf = CWrapperIf::getInstance().lock();
        if(!cWrapperIf)
        {
            return false;
        }
        if(!m_isLSockPathCreated)
        {
            auto rc = FileSystemIf::getInstance().lock()->createPath(ITC_PATH_SOCK_FOLDER_NAME, PathType::DIRECTORY, ITC_PATH_ITC_DIRECTORY_POSITION);
            if(rc != MAKE_RETURN_CODE(FileSystemIfReturnCode, ITC_FILESYSTEM_OK))
            {
                return false;
            }
            m_isLSockPathCreated = true;
        }
        
        auto sockFd = cWrapperIf->cSocket(AF_LOCAL, SOCK_STREAM, 0);
        if(sockFd < 0)
        {
            TPT_TRACE(TRACE_ERROR, SSTR("Failed to open LSocket!"));
            return false;
        }
        
        sockaddr_un itcServerAddr;
        cWrapperIf->cMemset(&itcServerAddr, 0, sizeof(sockaddr_un));
        itcServerAddr.sun_family = AF_LOCAL;
        ::sprintf(itcServerAddr.sun_path, "%s_0x%08x", ITC_PATH_LSOCK_BASE_FILE_NAME, regionId);
        
        /***
         * When using connect() remember always use sizeof(coord_addr) here. If you use sizeof(sockaddr) instead,
         * there will be a bug that "No such file or directory" even though it's existing, Idk why :)
         */
        int32_t res = cWrapperIf->cConnect(sockFd, (const sockaddr*)&itcServerAddr, sizeof(itcServerAddr));
        if(res < 0)
        {
            TPT_TRACE(TRACE_ERROR, SSTR("Failed to connect to address ", itcServerAddr.sun_path, ", res = ", res, ", errno = ", errno));
            return false;
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
        
        if(rxLen < 0)
        {
            TPT_TRACE(TRACE_ABN, SSTR("ACK from itc-server is not received, rxLen = ", rxLen));
            return false;
        } else if(rxLen != 4 || cWrapperIf->cStrcmp("ack", ack) != 0)
        {
            return false;
        }
        
        m_sockFd = sockFd;
        return true;
    }
    return false;
}

void ItcTransportLSocket::release()
{    
    if(m_isItcServerRunning)
    {
        auto ret = CWrapperIf::getInstance().lock()->cClose(m_sockFd);
        if(ret < 0)
        {
            TPT_TRACE(TRACE_ERROR, SSTR("Failed to close LSocket!"));
            return;
        }
        m_sockFd = -1;
        
        auto rc = FileSystemIf::getInstance().lock()->removePath(ITC_PATH_SOCK_FOLDER_NAME);
        if(rc != MAKE_RETURN_CODE(FileSystemIfReturnCode, ITC_FILESYSTEM_OK))
        {
            TPT_TRACE(TRACE_ERROR, SSTR("Failed to remove socket path ", ITC_PATH_SOCK_FOLDER_NAME, "!"));
            return;
        }
        m_isLSockPathCreated = false;
        m_isItcServerRunning = false;
    }
}

LocatedResults ItcTransportLSocket::locateItcServer()
{
    LocatedResults locatedResults;
    auto cWrapperIf = CWrapperIf::getInstance().lock();
    if(!cWrapperIf)
    {
        return locatedResults;
    }
    
    int32_t sockFd = cWrapperIf->cSocket(AF_LOCAL, SOCK_STREAM, 0);
	if(sockFd < 0)
	{
		TPT_TRACE(TRACE_ERROR, "Failed to call socket()!");
		return locatedResults;
	}
    
    struct sockaddr_un itcServerAddr;
    cWrapperIf->cMemset(&itcServerAddr, 0, sizeof(struct sockaddr_un));
    itcServerAddr.sun_family = AF_LOCAL;
	cWrapperIf->cStrcpy(itcServerAddr.sun_path, ITC_PATH_ITC_SERVER_SOCKET);
    
    int32_t ret = cWrapperIf->cConnect(sockFd, (struct sockaddr*)&itcServerAddr, sizeof(itcServerAddr));
	if(ret < 0)
	{
		TPT_TRACE(TRACE_ERROR, SSTR("Failed to connect(), errno = ", errno));
		return locatedResults;
	}

    itc_ethernet_message_locate_itc_server_request lrequest;
	lrequest.msgno  = ITC_ETHERNET_MESSAGE_LOCATE_ITC_SERVER_REQUEST;
	lrequest.pid    = cWrapperIf->cGetPid();
    
    ret = cWrapperIf->cSend(sockFd, &lrequest, sizeof(itc_ethernet_message_locate_itc_server_request), 0);
    if(ret < 0)
	{
		TPT_TRACE(TRACE_ERROR, SSTR("Failed to send ITC_ETHERNET_MESSAGE_LOCATE_ITC_SERVER_REQUEST!"));
		return locatedResults;
	}
    
    uint8_t rxBuffer[ITC_MAX_SOCKET_RX_BUFFER_SIZE];
    auto lreply = reinterpret_cast<itc_ethernet_message_locate_itc_server_reply *>(rxBuffer);
    auto receivedBytes = cWrapperIf->cRecv(sockFd, lreply, ITC_MAX_SOCKET_RX_BUFFER_SIZE, 0);
	if(receivedBytes < (ssize_t)sizeof(itc_ethernet_message_locate_itc_server_reply))
	{
		TPT_TRACE(TRACE_ABN, SSTR("Invalid ITC_ETHERNET_MESSAGE_LOCATE_ITC_SERVER_REQUEST received, receivedBytes = ", receivedBytes));
		return locatedResults;
	}
    
    cWrapperIf->cClose(sockFd);
    
    /* Done communication, start analyzing the response */
	if(lreply->msgno == ITC_ETHERNET_MESSAGE_LOCATE_ITC_SERVER_REPLY && lreply->assignedRegionId != ITC_MAILBOX_ID_DEFAULT)
	{
        locatedResults.assignedRegionId = lreply->assignedRegionId;
        locatedResults.itcServerMboxId = lreply->itcServerMboxId;
        m_isItcServerRunning = true;
	} else
	{
		/* Indicate that we have received a strange message type in response */
		TPT_TRACE(TRACE_ABN, SSTR("Unexpected ITC_ETHERNET_MESSAGE_LOCATE_ITC_SERVER_REPLY information received!"));
	}
    
    return locatedResults;
}

} // namespace INTERNAL
} // namespace ITC