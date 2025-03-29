#pragma once

#include <time.h>
#include <iostream>

#include "itcCWrapperIf.h"

// #include <traceIf.h>
// #include "itc-common/inc/itcTptProvider.h"

namespace ItcPlatform
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{
	
/***
 * TODO: For compilation only (TBD)
 */
#define TPT_TRACE(a, b)

unsigned long int getTimeDiffInNanoSec(struct timespec tStart, struct timespec tEnd);

#if defined MUTEX_TIME_MEASURE_DEBUG_ENABLE
/* Logging prolong locking mutexes which is longer than 5ms */
#define MUTEX_LOCK(mtx)									                    				\
	do											                               			 	\
	{											                                			\
		struct timespec tStart;							                    				\
		struct timespec tEnd;								                    			\
		CWrapperIf::getInstance().lock()->cClockGetTime(CLOCK_MONOTONIC, &tStart);			\
		CWrapperIf::getInstance().lock()->cPthreadMutexLock(mtx);							\
		CWrapperIf::getInstance().lock()->cClockGetTime(CLOCK_MONOTONIC, &tEnd);			\
		unsigned long int difftime = getTimeDiffInNanoSec(tStart, tEnd);					\
		if(difftime/1000000 > 5) UNLIKELY 							           				\
		{										                                			\
			TPT_TRACE(TRACE_DEBUG, SSTR("MUTEX_LOCK\t", std::hex,               			\
                (unsigned long)mtx, ",\t", __FILE__, ":", __LINE__,            				\
                ",\ttime_elapsed = ", difftime/1000000, " (ms)!"));	            			\
			TPT_TRACE(TRACE_DEBUG, SSTR("MUTEX_LOCK - tStart.tv_sec = ", 					\
				tStart.tv_sec));						                        			\
			TPT_TRACE(TRACE_DEBUG, SSTR("MUTEX_LOCK - tStart.tv_nsec = ", 					\
				tStart.tv_nsec));						                        			\
			TPT_TRACE(TRACE_DEBUG, SSTR("MUTEX_LOCK - tEnd.tv_sec = ", 						\
				tEnd.tv_sec));							                        			\
			TPT_TRACE(TRACE_DEBUG, SSTR("MUTEX_LOCK - tEnd.tv_nsec = ",						\
				tEnd.tv_nsec));							                    				\
		}										                                			\
	} while(0)

#define MUTEX_UNLOCK(mtx)									                    			\
	do											                                			\
	{											                                			\
		struct timespec tStart;							                    				\
		struct timespec tEnd;								                    			\
		CWrapperIf::getInstance().lock()->cClockGetTime(CLOCK_MONOTONIC, &tStart);			\
		CWrapperIf::getInstance().lock()->cPthreadMutexUnlock(mtx);							\
		CWrapperIf::getInstance().lock()->cClockGetTime(CLOCK_MONOTONIC, &tEnd);			\
		unsigned long int difftime = getTimeDiffInNanoSec(tStart, tEnd);					\
		if(difftime/1000000 > 5) UNLIKELY							          				\
		{										                                			\
			TPT_TRACE(TRACE_DEBUG, SSTR("MUTEX_UNLOCK\t", std::hex,             			\
                (unsigned long)mtx, ",\t", __FILE__, ":", __LINE__,            				\
                ",\ttime_elapsed = ", difftime/1000000, " (ms)!"));	            			\
			TPT_TRACE(TRACE_DEBUG, SSTR("MUTEX_UNLOCK - tStart.tv_sec = ", 					\
				tStart.tv_sec));						                        			\
			TPT_TRACE(TRACE_DEBUG, SSTR("MUTEX_UNLOCK - tStart.tv_nsec = ", 				\
				tStart.tv_nsec));						                        			\
			TPT_TRACE(TRACE_DEBUG, SSTR("MUTEX_UNLOCK - tEnd.tv_sec = ", 					\
				tEnd.tv_sec));							                        			\
			TPT_TRACE(TRACE_DEBUG, SSTR("MUTEX_UNLOCK - tEnd.tv_nsec = ",					\
				tEnd.tv_nsec));							                    				\
		}										                                			\
	} while(0)
#elif defined MUTEX_DEBUG_ENABLE
#define MUTEX_LOCK(mtx)								                        				\
	do										                                    			\
	{										                                    			\
		TPT_TRACE(TRACE_DEBUG, SSTR("MUTEX_LOCK\t", std::hex,                   			\
            (unsigned long)mtx, ",\t", __FILE__, ":", __LINE__));			    			\
		CWrapperIf::getInstance().lock()->cPthreadMutexLock(mtx);							\
	} while(0)

#define MUTEX_UNLOCK(mtx)								                        			\
	do										                                    			\
	{										                                    			\
		TPT_TRACE(TRACE_DEBUG, SSTR("MUTEX_UNLOCK\t", std::hex,                 			\
            (unsigned long)mtx, ",\t", __FILE__, ":", __LINE__));			    			\
		CWrapperIf::getInstance().lock()->cPthreadMutexUnlock(mtx);							\
	} while(0)
#else // defined MUTEX_DEBUG_ENABLE
#define MUTEX_LOCK(mtx)								                        				\
	do										                                    			\
	{										                                    			\
		CWrapperIf::getInstance().lock()->cPthreadMutexLock(mtx);							\
	} while(0)

#define MUTEX_UNLOCK(mtx)								                        			\
	do										                                    			\
	{										                                    			\
		CWrapperIf::getInstance().lock()->cPthreadMutexUnlock(mtx);							\
	} while(0)
#endif

} // namespace INTERNAL
} // namespace ItcPlatform