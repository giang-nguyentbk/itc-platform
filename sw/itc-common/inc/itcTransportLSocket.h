#pragma once

#include <string>

#include <cstdint>
#include <cstddef>
#include <memory>
#include <mutex>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <gtest/gtest.h>

// #include <enumUtils.h>

#include "itc.h"
#include "itcConstant.h"
#include "itcMailbox.h"
#include "itcAdminMessage.h"
#include "itcThreadManager.h"
#include "itcTransportIf.h"

namespace ItcPlatform
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{
using namespace ItcPlatform::PROVIDED;

static const std::string ITC_PATH_SOCK_FOLDER_NAME      {"/tmp/itc/socket"};
static const std::string ITC_PATH_LSOCK_BASE_FILE_NAME  {"/tmp/itc/socket/lsocket"};
static const std::string ITC_PATH_ITC_SERVER_FILE_NAME  {"/tmp/itc/itc-server/itc-server"};

/***
 * This transport is to locate itc-server only. The letter "l" in "lsocket" means locating.
 */
class ItcTransportLSocket : public ItcTransportIf
{
public:
    static std::weak_ptr<ItcTransportLSocket> getInstance();
    virtual ~ItcTransportLSocket() = default;
    
    ItcTransportLSocket(const ItcTransportLSocket &other) = delete;
    ItcTransportLSocket &operator=(const ItcTransportLSocket &other) = delete;
    ItcTransportLSocket(ItcTransportLSocket &&other) noexcept = delete;
    ItcTransportLSocket &operator=(ItcTransportLSocket &&other) noexcept = delete;
    
    ItcTransportIfReturnCode initialise(itc_mailbox_id_t regionId = ITC_NO_MAILBOX_ID, uint32_t mboxCount = 0, uint32_t flags = ITC_NO_MAILBOX_ID) override;
    ItcTransportIfReturnCode release() override;
    ItcTransportIfReturnCode locateItcServer(itc_mailbox_id_t *assignedRegionId, itc_mailbox_id_t *locatedItcServerMboxId) override;
    ItcTransportIfReturnCode createMailbox(ItcMailboxRawPtr newMbox) override;
    ItcTransportIfReturnCode deleteMailbox(ItcMailboxRawPtr mbox) override;
    ItcTransportIfReturnCode sendMessage(ItcAdminMessageRawPtr adminMsg, const MailboxContactInfo &toMbox) override;
    ItcAdminMessageRawPtr receiveMessage(ItcMailboxRawPtr myMbox) override;
    size_t getMaxMessageSize() override;

private:
    ItcTransportLSocket() = default;
    
    /***
     * @DEPRECATED
     */
    ItcTransportIfReturnCode createLSockPath();
    
private:
    static std::shared_ptr<ItcTransportLSocket> m_instance;
	static std::mutex m_singletonMutex;
    int32_t m_sockFd {-1};
    bool m_isItcServerRunning {false};
    bool m_isLSockPathCreated {false};
    
    friend class ItcTransportLSocketTest;
	FRIEND_TEST(ItcTransportLSocketTest, initialiseTest1);
	FRIEND_TEST(ItcTransportLSocketTest, initialiseTest2);
	FRIEND_TEST(ItcTransportLSocketTest, initialiseTest3);
	FRIEND_TEST(ItcTransportLSocketTest, initialiseTest4);
	FRIEND_TEST(ItcTransportLSocketTest, initialiseTest5);
	FRIEND_TEST(ItcTransportLSocketTest, initialiseTest6);
	FRIEND_TEST(ItcTransportLSocketTest, initialiseTest7);
	FRIEND_TEST(ItcTransportLSocketTest, initialiseTest8);
	FRIEND_TEST(ItcTransportLSocketTest, releaseTest1);
	FRIEND_TEST(ItcTransportLSocketTest, releaseTest2);
	FRIEND_TEST(ItcTransportLSocketTest, releaseTest3);
	FRIEND_TEST(ItcTransportLSocketTest, releaseTest4);
	FRIEND_TEST(ItcTransportLSocketTest, locateItcServerTest1);
	FRIEND_TEST(ItcTransportLSocketTest, locateItcServerTest2);
	FRIEND_TEST(ItcTransportLSocketTest, locateItcServerTest3);
	FRIEND_TEST(ItcTransportLSocketTest, locateItcServerTest4);
	FRIEND_TEST(ItcTransportLSocketTest, locateItcServerTest5);
	FRIEND_TEST(ItcTransportLSocketTest, locateItcServerTest6);
}; // class ItcTransportLSocket

} // namespace INTERNAL
} // namespace ItcPlatform