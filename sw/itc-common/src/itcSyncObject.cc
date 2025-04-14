#include "itcSyncObject.h"

namespace ITC
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

void SyncObject::calculateExpiredDate(uint32_t clockMode)
{
	auto cWrapperIf = CWrapperIf::getInstance().lock();
	cWrapperIf->cMemset(&timeout, 0, sizeof(struct timespec));
	cWrapperIf->cClockGetTime(clockMode, &timeout);
	timeout.tv_sec 		+= relativeTimeout/ 1000;
	timeout.tv_nsec 	+= (relativeTimeout % 1000) * 1000000;
	if(timeout.tv_nsec >= 1000000000)
	{
		timeout.tv_sec		+= 1;
		timeout.tv_nsec	-= 1000000000;
	}
}

void SyncObject::reset()
{
	if(!isInitialised)
	{
		return;
	}
	
	auto cWrapperIf = CWrapperIf::getInstance().lock();
	if(!cWrapperIf)
	{
		return;
	}
	int32_t ret = cWrapperIf->cPthreadCondAttrDestroy(&elems->condAttrs);
	if(ret != 0) UNLIKELY
	{
		TPT_TRACE(TRACE_ERROR, SSTR("Failed to pthread_condattr_destroy error code = ", ret));
	}
	
	ret = cWrapperIf->cPthreadMutexAttrDestroy(&elems->mtxAttrs);
	if(ret != 0) UNLIKELY
	{
		TPT_TRACE(TRACE_ERROR, SSTR("Failed to pthread_mutexattr_destroy error code = ", ret));
	}
	
	ret = cWrapperIf->cPthreadCondDestroy(&elems->cond);
	if(ret != 0) UNLIKELY
	{
		TPT_TRACE(TRACE_ERROR, SSTR("Failed to pthread_cond_destroy error code = ", ret));
	}
	
	ret = cWrapperIf->cPthreadMutexDestroy(&elems->mtx);
	if(ret != 0) UNLIKELY
	{
		TPT_TRACE(TRACE_ERROR, SSTR("Failed to pthread_mutex_destroy error code = ", ret));
	}
	
	if(elems->fd != -1)
	{
		cWrapperIf->cClose(elems->fd);
		elems->fd = -1;
	}
	
	isInitialised = false;
}

void SyncObject::init(SetupSyncObjectElemsAttrsFunc setupElemsAttrsFunc)
{
	auto cWrapperIf = CWrapperIf::getInstance().lock();
	if(!cWrapperIf)
	{
		return;
	}
	
	elems = std::make_shared<SyncObjectElements>();
	
	int32_t ret = cWrapperIf->cPthreadCondAttrInit(&elems->condAttrs);
	if(ret != 0) UNLIKELY
	{
		TPT_TRACE(TRACE_ERROR, SSTR("Failed to pthread_condattr_init, error code = ", ret));
		return;
	}
	
	ret = cWrapperIf->cPthreadMutexAttrInit(&elems->mtxAttrs);
	if(ret != 0) UNLIKELY
	{
		TPT_TRACE(TRACE_ERROR, SSTR("Failed to pthread_mutexattr_init, error code = ", ret));
		return;
	}
	
	ret = setupElemsAttrsFunc(elems);
	if(ret != 0) UNLIKELY
	{
		return;
	}
	
	ret = cWrapperIf->cPthreadCondInit(&elems->cond, &elems->condAttrs);
	if(ret != 0) UNLIKELY
	{
		TPT_TRACE(TRACE_ERROR, SSTR("Failed to pthread_cond_init, error code = ", ret));
		return;
	}
	
	ret = cWrapperIf->cPthreadMutexInit(&elems->mtx, &elems->mtxAttrs);
	if(ret != 0) UNLIKELY
	{
		TPT_TRACE(TRACE_ERROR, SSTR("Failed to pthread_mutex_init, error code = ", ret));
		return;
	}
	
	elems->fd = cWrapperIf->cEventFd(0, 0);
	if(elems->fd == -1)
	{
		TPT_TRACE(TRACE_ERROR, "Failed to eventfd()!");
		return;
	}
	
	/* Initial trigger to allow current thread to check and clean up the resources
	that event fd is waiting for */
	if(elems->fdEventCounter)
	{
		int64_t one = 1;
		if(cWrapperIf->cWrite(elems->fd, &one, sizeof(uint64_t)) < 0)
        {
            TPT_TRACE(TRACE_ERROR, "Failed to write()!");
			return;
        }
	}
	
	isInitialised = true;
}

} // namespace INTERNAL
} // namespace ITC