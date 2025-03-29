#pragma once

#include <cstdint>
#include <cstddef>

// #include <enumUtils.h>

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

class ItcTransportIf
{
public:
    ItcTransportIf(const ItcTransportIf &other) = delete;
    ItcTransportIf &operator=(const ItcTransportIf &other) = delete;
    ItcTransportIf(ItcTransportIf &&other) noexcept = delete;
    ItcTransportIf &operator=(ItcTransportIf &&other) noexcept = delete;
    
	enum class ItcTransportIfReturnCode
	{
		ITC_TRANSPORT_UNDEFINED,
		ITC_TRANSPORT_OK,
		ITC_TRANSPORT_ALREADY_INITIALISED,
		ITC_TRANSPORT_NOT_INITIALISED,
		ITC_TRANSPORT_INVALID_ARGUMENTS,
		ITC_TRANSPORT_INVALID_REGION,
		ITC_TRANSPORT_OUT_OF_RANGE,
		ITC_TRANSPORT_INVALID_OPERATION,
		ITC_TRANSPORT_ALREADY_IN_USE,
		ITC_TRANSPORT_SYSCALL_ERROR,
		ITC_TRANSPORT_UNEXPECTED_RESPONSE,
		ITC_TRANSPORT_ITC_SERVER_NOT_RUNNING,
		ITC_TRANSPORT_INVALID_RESPONSE,
	};
	
	// enum class ItcTransportIfReturnCodeRaw
	// {
	// 	ITC_TRANSPORT_UNDEFINED,
	// 	ITC_TRANSPORT_OK,
	// 	ITC_TRANSPORT_ALREADY_INITIALISED,
	// 	ITC_TRANSPORT_NOT_INITIALISED,
	// 	ITC_TRANSPORT_INVALID_REGION,
	// 	ITC_TRANSPORT_OUT_OF_RANGE,
	// 	ITC_TRANSPORT_IN_WRONG_STATE,
	// 	ITC_TRANSPORT_ALREADY_IN_USE,
	// 	ITC_TRANSPORT_SYSCALL_ERROR,
	// 	ITC_TRANSPORT_UNEXPECTED_RESPONSE,
	// 	ITC_TRANSPORT_ITC_SERVER_NOT_RUNNING,
	// };

	// class ItcTransportIfReturnCode : public EnumType<ItcTransportIfReturnCodeRaw>
	// {
	// public:
	// 	explicit ItcTransportIfReturnCode(const ItcTransportIfReturnCodeRaw& raw) : EnumType<ItcTransportIfReturnCodeRaw>(raw) {}
	// 	explicit ItcTransportIfReturnCode() : EnumType<ItcTransportIfReturnCodeRaw>(ItcTransportIfReturnCodeRaw::ITC_TRANSPORT_UNDEFINED) {}

	// 	std::string toString() const override
	// 	{
	// 		switch (getRawEnum())
	// 		{
	// 		case ItcTransportIfReturnCodeRaw::ITC_TRANSPORT_UNDEFINED:
	// 			return "ITC_TRANSPORT_UNDEFINED";

	// 		case ItcTransportIfReturnCodeRaw::ITC_TRANSPORT_OK:
	// 			return "ITC_TRANSPORT_OK";

	// 		case ItcTransportIfReturnCodeRaw::ITC_TRANSPORT_ALREADY_INITIALISED:
	// 			return "ITC_TRANSPORT_ALREADY_INITIALISED";
			
	// 		case ItcTransportIfReturnCodeRaw::ITC_TRANSPORT_NOT_INITIALISED:
	// 			return "ITC_TRANSPORT_NOT_INITIALISED";

	// 		default:
	// 			return "Unknown EnumType: " + std::to_string(toS32());
	// 		}
	// 	}
	// };
	
    virtual ItcTransportIfReturnCode initialise(itc_mailbox_id_t regionId = ITC_NO_MAILBOX_ID, uint32_t mboxCount = 0, uint32_t flags = ITC_NO_MAILBOX_ID) = 0;
    virtual ItcTransportIfReturnCode release() = 0;
    virtual ItcTransportIfReturnCode locateItcServer(itc_mailbox_id_t *assignedRegionId, itc_mailbox_id_t *locatedItcServerMboxId) = 0;
    virtual ItcTransportIfReturnCode createMailbox(ItcMailboxRawPtr newMbox) = 0;
    virtual ItcTransportIfReturnCode deleteMailbox(ItcMailboxRawPtr mbox) = 0;
    virtual ItcTransportIfReturnCode sendMessage(ItcAdminMessageRawPtr adminMsg, const MailboxContactInfo &toMbox) = 0;
    virtual ItcAdminMessageRawPtr receiveMessage(ItcMailboxRawPtr myMbox) = 0;
    virtual size_t getMaxMessageSize() = 0;
    
protected:
    ItcTransportIf() = default;
    virtual ~ItcTransportIf() = default;
}; // class ItcTransportIf

} // namespace INTERNAL

} // namespace ItcPlatform