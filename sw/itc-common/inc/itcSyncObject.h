#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <functional>
#include <thread>

#include "itc.h"
#include "itcCWrapperIf.h"


namespace ITC
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

struct SyncObjectElements
{
	pthread_cond_t 			cond;
	pthread_condattr_t		condAttrs;
	pthread_mutex_t			mtx;
	pthread_mutexattr_t		mtxAttrs;
	int32_t					fd {-1};
	uint32_t				fdEventCounter {0};
};

using SyncObjectElementsSharedPtr = std::shared_ptr<SyncObjectElements>;
using SetupSyncObjectElemsAttrsFunc = std::function<int32_t(SyncObjectElementsSharedPtr elemsPtr)>;

struct SyncObject
{
	SyncObjectElementsSharedPtr		elems;
	struct timespec 				timeout;
	
	explicit SyncObject(SetupSyncObjectElemsAttrsFunc setupElemsAttrsFunc)
	{
		init(setupElemsAttrsFunc);
	}
	virtual ~SyncObject()
	{
		reset();
	}
	
	SyncObject(const SyncObject &other) = default;
	SyncObject &operator=(const SyncObject &other) = default;
	SyncObject(SyncObject &&other) noexcept = default;
	SyncObject &operator=(SyncObject &&other) noexcept = default;

	void setTimeout(uint32_t iTimeout /* ms */)
	{
		relativeTimeout = iTimeout;
	}
	
	void calculateExpiredDate(uint32_t clockMode = CLOCK_MONOTONIC);
	
	void reset();
	
private:
	void init(SetupSyncObjectElemsAttrsFunc setupElemsAttrsFunc);

private:
	uint32_t relativeTimeout {0};
	bool isInitialised {false};
};


} // namespace INTERNAL
} // namespace ITC