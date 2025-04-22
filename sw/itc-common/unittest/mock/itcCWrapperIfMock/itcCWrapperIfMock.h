#pragma once

#include "itcCWrapperIf.h"
#include "itcConstant.h"

#include <gmock/gmock.h>

namespace ITC
{
namespace INTERNAL
{

class CWrapperIfMock : public CWrapperIf
{
public:
    static std::weak_ptr<CWrapperIfMock> getInstance();
    virtual ~CWrapperIfMock() = default;
    
    // First prevent copy/move construtors/assignments
	CWrapperIfMock(const CWrapperIfMock&)               = delete;
	CWrapperIfMock(CWrapperIfMock&&)                    = delete;
	CWrapperIfMock& operator=(const CWrapperIfMock&)    = delete;
	CWrapperIfMock& operator=(CWrapperIfMock&&)         = delete;

    /***
     * Network APIs
     */
    MOCK_METHOD(int32_t, cSocket, (int32_t domain, int32_t type, int32_t protocol), (override));
    MOCK_METHOD(int32_t, cConnect, (int32_t sockfd, const struct sockaddr *addr, socklen_t addrlen), (override));
    MOCK_METHOD(ssize_t, cSend, (int32_t sockfd, const void *buf, size_t size, int32_t flags), (override));
    MOCK_METHOD(ssize_t, cRecv, (int32_t sockfd, void *buf, size_t size, int32_t flags), (override));
    // MOCK_METHOD(int32_t, cClose, (int32_t fd), (override));
    int32_t cClose(int32_t fd)
    {
        return ::close(fd);
    }
    
    /***
     * Threading APIs
     */
    // MOCK_METHOD(int32_t, cPthreadCondAttrInit, (pthread_condattr_t *attr), (override));
    int32_t cPthreadCondAttrInit(pthread_condattr_t *attr)
    {
        return ::pthread_condattr_init(attr);
    }
    // MOCK_METHOD(int32_t, cPthreadMutexAttrInit, (pthread_mutexattr_t *attr), (override));
    int32_t cPthreadMutexAttrInit(pthread_mutexattr_t *attr)
    {
        return ::pthread_mutexattr_init(attr);
    }
    // MOCK_METHOD(int32_t, cPthreadMutexAttrSetType, (pthread_mutexattr_t *attr, int32_t type), (override));
    int32_t cPthreadMutexAttrSetType(pthread_mutexattr_t *attr, int32_t type)
    {
        return ::pthread_mutexattr_settype(attr, type);
    }
    // MOCK_METHOD(int32_t, cPthreadCondAttrSetClock, (pthread_condattr_t *attr, clockid_t clock_id), (override));
    int32_t cPthreadCondAttrSetClock(pthread_condattr_t *attr, clockid_t clock_id)
    {
        return ::pthread_condattr_setclock(attr, clock_id);
    }
    // MOCK_METHOD(int32_t, cPthreadCondInit, (pthread_cond_t *cond, const pthread_condattr_t *attr), (override));
    int32_t cPthreadCondInit(pthread_cond_t *cond, const pthread_condattr_t *attr)
    {
        return ::pthread_cond_init(cond, attr);
    }
    // MOCK_METHOD(int32_t, cPthreadMutexInit, (pthread_mutex_t *mutex, const pthread_mutexattr_t *attr), (override));
    int32_t cPthreadMutexInit(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
    {
        return ::pthread_mutex_init(mutex, attr);
    }
    // MOCK_METHOD(int32_t, cPthreadCondAttrDestroy, (pthread_condattr_t *attr), (override));
    int32_t cPthreadCondAttrDestroy(pthread_condattr_t *attr)
    {
        return ::pthread_condattr_destroy(attr);
    }
    // MOCK_METHOD(int32_t, cPthreadMutexAttrDestroy, (pthread_mutexattr_t *attr), (override));
    int32_t cPthreadMutexAttrDestroy(pthread_mutexattr_t *attr)
    {
        return ::pthread_mutexattr_destroy(attr);
    }
    // MOCK_METHOD(int32_t, cPthreadCondDestroy, (pthread_cond_t *cond), (override));
    int32_t cPthreadCondDestroy(pthread_cond_t *cond)
    {
        return ::pthread_cond_destroy(cond);
    }
    // MOCK_METHOD(int32_t, cPthreadMutexDestroy, (pthread_mutex_t *mutex), (override));
    int32_t cPthreadMutexDestroy(pthread_mutex_t *mutex)
    {
        return ::pthread_mutex_destroy(mutex);
    }
    // MOCK_METHOD(int32_t, cPthreadCreate, (pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg), (override));
    int32_t cPthreadCreate(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg)
    {
        return ::pthread_create(thread, attr, start_routine, arg);
    }
    // MOCK_METHOD(int32_t, cPthreadCancel, (pthread_t thread), (override));
    int32_t cPthreadCancel(pthread_t thread)
    {
        return ::pthread_cancel(thread);
    }
    // MOCK_METHOD(int32_t, cPthreadJoin, (pthread_t thread, void **retval), (override));
    int32_t cPthreadJoin(pthread_t thread, void **retval)
    {
        return ::pthread_join(thread, retval);
    }
    // MOCK_METHOD(int32_t, cPthreadDetach, (pthread_t thread), (override));
    int32_t cPthreadDetach(pthread_t thread)
    {
        return ::pthread_detach(thread);
    }
    // MOCK_METHOD(int32_t, cPthreadMutexLock, (pthread_mutex_t *mutex), (override));
    int32_t cPthreadMutexLock(pthread_mutex_t *mutex)
    {
        return ::pthread_mutex_lock(mutex);
    }
    // MOCK_METHOD(int32_t, cPthreadMutexUnlock, (pthread_mutex_t *mutex), (override));
    int32_t cPthreadMutexUnlock(pthread_mutex_t *mutex)
    {
        return ::pthread_mutex_unlock(mutex);
    }
    // MOCK_METHOD(int32_t, cPthreadCondTimedWait, (pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime), (override));
    int32_t cPthreadCondTimedWait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime)
    {
        return ::pthread_cond_timedwait(cond, mutex, abstime);
    }
    // MOCK_METHOD(int32_t, cPthreadCondWait, (pthread_cond_t *cond, pthread_mutex_t *mutex), (override));
    int32_t cPthreadCondWait(pthread_cond_t *cond, pthread_mutex_t *mutex)
    {
        return ::pthread_cond_wait(cond, mutex);
    }
    // MOCK_METHOD(int32_t, cPthreadCondSignal, (pthread_cond_t *cond), (override));
    int32_t cPthreadCondSignal(pthread_cond_t *cond)
    {
        return ::pthread_cond_signal(cond);
    }
    // MOCK_METHOD(int32_t, cPthreadCondBroadcast, (pthread_cond_t *cond), (override));
    int32_t cPthreadCondBroadcast(pthread_cond_t *cond)
    {
        return ::pthread_cond_broadcast(cond);
    }
    MOCK_METHOD(int32_t, cPthreadKeyCreate, (pthread_key_t *key, void (*destructor)(void*)), (override));
    // int32_t cPthreadKeyCreate(pthread_key_t *key, void (*destructor)(void*))
    // {
    //     return ::pthread_key_create(key, destructor);
    // }
    MOCK_METHOD(int32_t, cPthreadKeyDelete, (pthread_key_t key), (override));
    // int32_t cPthreadKeyDelete(pthread_key_t key)
    // {
    //     return ::pthread_key_delete(key);
    // }
    // MOCK_METHOD(int32_t, cPthreadAttrInit, (pthread_attr_t *attr), (override));
    int32_t cPthreadAttrInit(pthread_attr_t *attr)
    {
        return ::pthread_attr_init(attr);
    }
    // MOCK_METHOD(int32_t, cPthreadAttrDestroy, (pthread_attr_t *attr), (override));
    int32_t cPthreadAttrDestroy(pthread_attr_t *attr)
    {
        return ::pthread_attr_destroy(attr);
    }
    // MOCK_METHOD(int32_t, cPthreadSetSpecific, (pthread_key_t key, const void *value), (override));
    int32_t cPthreadSetSpecific(pthread_key_t key, const void *value)
    {
        return ::pthread_setspecific(key, value);
    }
    // MOCK_METHOD(int32_t, cPthreadSetCancelState, (int32_t state, int32_t *oldstate), (override));
    int32_t cPthreadSetCancelState(int32_t state, int32_t *oldstate)
    {
        return ::pthread_setcancelstate(state, oldstate);
    }
    // MOCK_METHOD(pid_t, cGetPid, (), (override));
    pid_t cGetPid()
    {
        return ::getpid();
    }
    // MOCK_METHOD(pid_t, cGetPPid, (), (override));
    pid_t cGetPPid()
    {
        return ::getppid();
    }

    /***
     * Process Scheduling APIs
     */
    // MOCK_METHOD(int32_t, cSchedGetPriorityMax, (int32_t policy), (override));
    int32_t cSchedGetPriorityMax(int32_t policy)
    {
        return ::sched_get_priority_max(policy);
    }
    // MOCK_METHOD(int32_t, cSchedGetPriorityMin, (int32_t policy), (override));
    int32_t cSchedGetPriorityMin(int32_t policy)
    {
        return ::sched_get_priority_min(policy);
    }
    // MOCK_METHOD(int32_t, cPthreadAttrSetSchedPolicy, (pthread_attr_t *attr, int32_t policy), (override));
    int32_t cPthreadAttrSetSchedPolicy(pthread_attr_t *attr, int32_t policy)
    {
        return ::pthread_attr_setschedpolicy(attr, policy);
    }
    // MOCK_METHOD(int32_t, cPthreadAttrSetSchedParam, (pthread_attr_t *attr, const struct sched_param *param), (override));
    int32_t cPthreadAttrSetSchedParam(pthread_attr_t *attr, const struct sched_param *param)
    {
        return ::pthread_attr_setschedparam(attr, param);
    }
    // MOCK_METHOD(int32_t, cPthreadAttrSetInheritSched, (pthread_attr_t *attr, int32_t inheritsched), (override));
    int32_t cPthreadAttrSetInheritSched(pthread_attr_t *attr, int inheritsched)
    {
        return ::pthread_attr_setinheritsched(attr, inheritsched);
    }

    /***
     * Filesystem APIs
     */
    // MOCK_METHOD(int32_t, cAccess, (const char *pathname, int32_t mode), (override));
    int32_t cAccess(const char *pathname, int32_t mode) override
    {
        return ::access(pathname, mode);
    }
    // MOCK_METHOD(int32_t, cMkdir, (const char *pathname, mode_t mode), (override));
    int32_t cMkdir(const char *pathname, mode_t mode)
    {
        return ::mkdir(pathname, mode);
    }
    // MOCK_METHOD(int32_t, cChmod, (const char *pathname, mode_t mode), (override));
    int32_t cChmod(const char *pathname, mode_t mode)
    {
        return ::chmod(pathname, mode);
    }
    // MOCK_METHOD(FILE *, cFopen, (const char *pathname, const char *mode), (override));
    FILE *cFopen(const char *pathname, const char *mode)
    {
        return ::fopen(pathname, mode);
    }
    // MOCK_METHOD(int32_t, cFclose, (FILE *stream), (override));
    int32_t cFclose(FILE *stream)
    {
        return ::fclose(stream);
    }
    // MOCK_METHOD(int32_t, cEventFd, (uint32_t initval, int32_t flags), (override));
    int32_t cEventFd(uint32_t initval, int32_t flags)
    {
        return ::eventfd(initval, flags);
    }
    // MOCK_METHOD(ssize_t, cRead, (int32_t fd, void *buf, size_t count), (override));
    ssize_t cRead(int32_t fd, void *buf, size_t count)
    {
        return ::read(fd, buf, count);
    }
    // MOCK_METHOD(ssize_t, cWrite, (int32_t fd, const void *buf, size_t count), (override));
    ssize_t cWrite(int32_t fd, const void *buf, size_t count)
    {
        return ::write(fd, buf, count);
    }
    
    /***
     * Time APIs
     */
    // MOCK_METHOD(int32_t, cClockGetTime, (clockid_t clockid, struct timespec *tp), (override));
    int32_t cClockGetTime(clockid_t clockid, struct timespec *tp)
    {
        return ::clock_gettime(clockid, tp);
    }
    // MOCK_METHOD(int32_t, cClockSetTime, (clockid_t clockid, const struct timespec *tp), (override));
    int32_t cClockSetTime(clockid_t clockid, const struct timespec *tp)
    {
        return ::clock_settime(clockid, tp);
    }
    // MOCK_METHOD(int32_t, cUsleep, (useconds_t usec), (override));
    int32_t cUsleep(useconds_t usec)
    {
        return ::usleep(usec);
    }
    // MOCK_METHOD(uint32_t, cSleep, (uint32_t seconds), (override));
    uint32_t cSleep(uint32_t seconds)
    {
        return ::sleep(seconds);
    }
    
    /***
     * String and memory APIs
     */
    // MOCK_METHOD(void *, cMemset, (void *s, int32_t c, size_t n), (override));
    void *cMemset(void *s, int32_t c, size_t n)
    {
        return ::memset(s, c, n);
    }
    // MOCK_METHOD(void *, cMemcpy, (void *dest, const void *src, size_t n), (override));
    void *cMemcpy(void *dest, const void *src, size_t n)
    {
        return ::memcpy(dest, src, n);
    }
    // MOCK_METHOD(int32_t, cStrcmp, (const char *s1, const char *s2), (override));
    int32_t cStrcmp(const char *s1, const char *s2)
    {
        return ::strcmp(s1, s2);
    }
    // MOCK_METHOD(char *, cStrcpy, (char *dst, const char *src), (override));
    char *cStrcpy(char *dst, const char *src)
    {
        return ::strcpy(dst, src);
    }
    // MOCK_METHOD(size_t, cStrlen, (const char *s), (override));
    size_t cStrlen(const char *s)
    {
        return ::strlen(s);
    }
    
    /***
     * SYSV Message Queue APIs
     */
    MOCK_METHOD(key_t, cFtok, (const char *pathname, int32_t proj_id), (override));
    MOCK_METHOD(int32_t, cMsgGet, (key_t key, int msgflg), (override));
    MOCK_METHOD(int32_t, cMsgctl, (int32_t msqid, int32_t op, struct msqid_ds *buf), (override));
    MOCK_METHOD(ssize_t, cMsgrcv, (int32_t msqid, void *msgp, size_t msgsz, long msgtyp, int32_t msgflg), (override));
    MOCK_METHOD(int32_t, cMsgsnd, (int32_t msqid, const void *msgp, size_t msgsz, int32_t msgflg), (override));
    
private:
    CWrapperIfMock() = default;

private:
    SINGLETON_DECLARATION(CWrapperIfMock)
    
    friend class FileSystemIfTest;
    friend class ThreadManagerIfTest;
    friend class ItcTransportLSocketTest;
    friend class ItcTransportSysvMsgQueueTest;
    
}; // class CWrapperIfMock

} // namespace INTERNAL
} // namespace ITC