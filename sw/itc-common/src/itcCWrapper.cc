#include "itcCWrapper.h"

#include <iostream>

#include <cstdint>
#include <cstddef>

#include <errno.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <sys/ipc.h>
#include <sys/msg.h>

#include "itcConstant.h"

// #include <traceIf.h>
// #include "itc-common/inc/itcTptProvider.h"

namespace ITC
{
namespace INTERNAL
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
SINGLETON_IF_DEFINITION(CWrapperIf, CWrapper)
#endif

SINGLETON_DEFINITION(CWrapper)

/***
 * Network APIs
 */
int32_t CWrapper::cSocket(int32_t domain, int32_t type, int32_t protocol)
{
    return ::socket(domain, type, protocol);
}

int32_t CWrapper::cConnect(int32_t sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    return ::connect(sockfd, addr, addrlen);
}

ssize_t CWrapper::cSend(int32_t sockfd, const void *buf, size_t size, int32_t flags)
{
    return ::send(sockfd, buf, size, flags);
}

ssize_t CWrapper::cRecv(int32_t sockfd, void *buf, size_t size, int32_t flags)
{
    return ::recv(sockfd, buf, size, flags);
}

int32_t CWrapper::cClose(int32_t fd)
{
    return ::close(fd);
}

/***
 * Threading APIs
 */
int32_t CWrapper::cPthreadCondAttrInit(pthread_condattr_t *attr)
{
    return ::pthread_condattr_init(attr);
}

int32_t CWrapper::cPthreadMutexAttrInit(pthread_mutexattr_t *attr)
{
    return ::pthread_mutexattr_init(attr);
}

int32_t CWrapper::cPthreadMutexAttrSetType(pthread_mutexattr_t *attr, int32_t type)
{
    return ::pthread_mutexattr_settype(attr, type);
}
int32_t CWrapper::cPthreadCondAttrSetClock(pthread_condattr_t *attr, clockid_t clock_id)
{
    return ::pthread_condattr_setclock(attr, clock_id);
}

int32_t CWrapper::cPthreadCondInit(pthread_cond_t *cond, const pthread_condattr_t *attr)
{
    return ::pthread_cond_init(cond, attr);
}

int32_t CWrapper::cPthreadMutexInit(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
    return ::pthread_mutex_init(mutex, attr);
}

int32_t CWrapper::cPthreadCondAttrDestroy(pthread_condattr_t *attr)
{
    return ::pthread_condattr_destroy(attr);
}

int32_t CWrapper::cPthreadMutexAttrDestroy(pthread_mutexattr_t *attr)
{
    return ::pthread_mutexattr_destroy(attr);
}

int32_t CWrapper::cPthreadCondDestroy(pthread_cond_t *cond)
{
    return ::pthread_cond_destroy(cond);
}

int32_t CWrapper::cPthreadMutexDestroy(pthread_mutex_t *mutex)
{
    return ::pthread_mutex_destroy(mutex);
}

int32_t CWrapper::cPthreadCreate(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg)
{
    return ::pthread_create(thread, attr, start_routine, arg);
}

int32_t CWrapper::cPthreadCancel(pthread_t thread)
{
    return ::pthread_cancel(thread);
}

int32_t CWrapper::cPthreadJoin(pthread_t thread, void **retval)
{
    return ::pthread_join(thread, retval);
}

int32_t CWrapper::cPthreadDetach(pthread_t thread)
{
    return ::pthread_detach(thread);
}

int32_t CWrapper::cPthreadMutexLock(pthread_mutex_t *mutex)
{
    return ::pthread_mutex_lock(mutex);
}

int32_t CWrapper::cPthreadMutexUnlock(pthread_mutex_t *mutex)
{
    return ::pthread_mutex_unlock(mutex);
}

int32_t CWrapper::cPthreadCondTimedWait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime)
{
    return ::pthread_cond_timedwait(cond, mutex, abstime);
}

int32_t CWrapper::cPthreadCondWait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    return ::pthread_cond_wait(cond, mutex);
}

int32_t CWrapper::cPthreadCondSignal(pthread_cond_t *cond)
{
    return ::pthread_cond_signal(cond);
}

int32_t CWrapper::cPthreadCondBroadcast(pthread_cond_t *cond)
{
    return ::pthread_cond_broadcast(cond);
}

int32_t CWrapper::cPthreadKeyCreate(pthread_key_t *key, void (*destructor)(void*))
{
    return ::pthread_key_create(key, destructor);
}

int32_t CWrapper::cPthreadKeyDelete(pthread_key_t key)
{
    return ::pthread_key_delete(key);
}

int32_t CWrapper::cPthreadAttrInit(pthread_attr_t *attr)
{
    return ::pthread_attr_init(attr);
}

int32_t CWrapper::cPthreadAttrDestroy(pthread_attr_t *attr)
{
    return ::pthread_attr_destroy(attr);
}

int32_t CWrapper::cPthreadSetSpecific(pthread_key_t key, const void *value)
{
    return ::pthread_setspecific(key, value);
}

int32_t CWrapper::cPthreadSetCancelState(int32_t state, int32_t *oldstate)
{
    return ::pthread_setcancelstate(state, oldstate);
}

pid_t CWrapper::cGetPid()
{
    return ::getpid();
}

pid_t CWrapper::cGetPPid()
{
    return ::getppid();
}



/***
 * Process Scheduling APIs
 */
int32_t CWrapper::cSchedGetPriorityMax(int32_t policy)
{
    return ::sched_get_priority_max(policy);
}

int32_t CWrapper::cSchedGetPriorityMin(int32_t policy)
{
    return ::sched_get_priority_min(policy);
}

int32_t CWrapper::cPthreadAttrSetSchedPolicy(pthread_attr_t *attr, int32_t policy)
{
    return ::pthread_attr_setschedpolicy(attr, policy);
}

int32_t CWrapper::cPthreadAttrSetSchedParam(pthread_attr_t *attr, const struct sched_param *param)
{
    return ::pthread_attr_setschedparam(attr, param);
}

int32_t CWrapper::cPthreadAttrSetInheritSched(pthread_attr_t *attr, int inheritsched)
{
    return ::pthread_attr_setinheritsched(attr, inheritsched);
}


/***
 * Filesystem APIs
 */
int32_t CWrapper::cAccess(const char *pathname, int32_t mode)
{
    return ::access(pathname, mode);
}

int32_t CWrapper::cMkdir(const char *pathname, mode_t mode)
{
    return ::mkdir(pathname, mode);
}

int32_t CWrapper::cChmod(const char *pathname, mode_t mode)
{
    return ::chmod(pathname, mode);
}

FILE *CWrapper::cFopen(const char *pathname, const char *mode)
{
    return ::fopen(pathname, mode);
}

int32_t CWrapper::cFclose(FILE *stream)
{
    return ::fclose(stream);
}


/***
 * Time APIs
 */
int32_t CWrapper::cClockGetTime(clockid_t clockid, struct timespec *tp)
{
    return ::clock_gettime(clockid, tp);
}

int32_t CWrapper::cClockSetTime(clockid_t clockid, const struct timespec *tp)
{
    return ::clock_settime(clockid, tp);
}

int32_t CWrapper::cUsleep(useconds_t usec)
{
    return ::usleep(usec);
}

uint32_t CWrapper::cSleep(uint32_t seconds)
{
    return ::sleep(seconds);
}


/***
 * String and memory APIs
 */
void *CWrapper::cMemset(void *s, int32_t c, size_t n)
{
    return ::memset(s, c, n);
}

void *CWrapper::cMemcpy(void *dest, const void *src, size_t n)
{
    return ::memcpy(dest, src, n);
}

int32_t CWrapper::cStrcmp(const char *s1, const char *s2)
{
    return ::strcmp(s1, s2);
}

char *CWrapper::cStrcpy(char *dst, const char *src)
{
    return ::strcpy(dst, src);
}

size_t CWrapper::cStrlen(const char *s)
{
    return ::strlen(s);
}


/***
 * SYSV Message Queue APIs
 */
key_t CWrapper::cFtok(const char *pathname, int32_t proj_id)
{
    return ::ftok(pathname, proj_id);
}

int32_t CWrapper::cMsgGet(key_t key, int msgflg)
{
    return ::msgget(key, msgflg);
}

int32_t CWrapper::cMsgctl(int32_t msqid, int32_t op, struct msqid_ds *buf)
{
    return ::msgctl(msqid, op, buf);
}

ssize_t CWrapper::cMsgrcv(int32_t msqid, void *msgp, size_t msgsz, long msgtyp, int32_t msgflg)
{
    return ::msgrcv(msqid, msgp, msgsz, msgtyp, msgflg);
}

int32_t CWrapper::cMsgsnd(int32_t msqid, const void *msgp, size_t msgsz, int32_t msgflg)
{
    return ::msgsnd(msqid, msgp, msgsz, msgflg);
}

} // namespace INTERNAL
} // namespace ITC
