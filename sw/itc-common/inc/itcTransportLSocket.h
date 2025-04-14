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

namespace ITC
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{
using namespace ITC::PROVIDED;

#define ITC_PATH_SOCK_FOLDER_NAME      		"/tmp/itc/socket"
#define ITC_PATH_LSOCK_BASE_FILE_NAME  		"/tmp/itc/socket/lsocket"

struct LocatedResults
{
    itc_mailbox_id_t assignedRegionId {ITC_MAILBOX_ID_DEFAULT};
    itc_mailbox_id_t itcServerMboxId {ITC_MAILBOX_ID_DEFAULT};
};

/***
 * This transport is to locate itc-server only. The letter "l" in "lsocket" means locating.
 */
class ItcTransportLSocket
{
public:
    static std::weak_ptr<ItcTransportLSocket> getInstance();
    virtual ~ItcTransportLSocket() = default;
    
    ItcTransportLSocket(const ItcTransportLSocket &other) = delete;
    ItcTransportLSocket &operator=(const ItcTransportLSocket &other) = delete;
    ItcTransportLSocket(ItcTransportLSocket &&other) noexcept = delete;
    ItcTransportLSocket &operator=(ItcTransportLSocket &&other) noexcept = delete;
    
    bool initialise(itc_mailbox_id_t regionId = ITC_MAILBOX_ID_DEFAULT);
    void release();
    LocatedResults locateItcServer();

private:
    ItcTransportLSocket() = default;
    
private:
    SINGLETON_DECLARATION(ItcTransportLSocket)
    
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
} // namespace ITC