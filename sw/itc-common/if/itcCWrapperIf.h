#pragma once

#include <memory>

#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <cstring>

#include <sys/ipc.h>
#include <sys/msg.h>

namespace ItcPlatform
{
namespace INTERNAL
{

class CWrapperIf
{
public:
    static std::weak_ptr<CWrapperIf> getInstance();
    
    // First prevent copy/move construtors/assignments
	CWrapperIf(const CWrapperIf&)               = delete;
	CWrapperIf(CWrapperIf&&)                    = delete;
	CWrapperIf& operator=(const CWrapperIf&)    = delete;
	CWrapperIf& operator=(CWrapperIf&&)         = delete;
    
    /***
     * Network APIs
     */
    virtual int32_t cSocket(int32_t domain, int32_t type, int32_t protocol) = 0;
    virtual int32_t cConnect(int32_t sockfd, const struct sockaddr *addr, socklen_t addrlen) = 0;
    virtual ssize_t cSend(int32_t sockfd, const void *buf, size_t size, int32_t flags) = 0;
    virtual ssize_t cRecv(int32_t sockfd, void *buf, size_t size, int32_t flags) = 0;
    virtual int32_t cClose(int32_t fd) = 0;
    
    /***
     * Threading APIs
     */
    virtual int32_t cPthreadCondAttrInit(pthread_condattr_t *attr) = 0;
    virtual int32_t cPthreadMutexAttrInit(pthread_mutexattr_t *attr) = 0;
    virtual int32_t cPthreadCondAttrSetClock(pthread_condattr_t *attr, clockid_t clock_id) = 0;
    virtual int32_t cPthreadCondInit(pthread_cond_t *cond, const pthread_condattr_t *attr) = 0;
    virtual int32_t cPthreadMutexInit(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr) = 0;
    virtual int32_t cPthreadCondAttrDestroy(pthread_condattr_t *attr) = 0;
    virtual int32_t cPthreadMutexAttrDestroy(pthread_mutexattr_t *attr) = 0;
    virtual int32_t cPthreadCondDestroy(pthread_cond_t *cond) = 0;
    virtual int32_t cPthreadMutexDestroy(pthread_mutex_t *mutex) = 0;
    virtual int32_t cPthreadCreate(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg) = 0;
    virtual int32_t cPthreadCancel(pthread_t thread) = 0;
    virtual int32_t cPthreadJoin(pthread_t thread, void **retval) = 0;
    virtual int32_t cPthreadDetach(pthread_t thread) = 0;
    virtual int32_t cPthreadMutexLock(pthread_mutex_t *mutex) = 0;
    virtual int32_t cPthreadMutexUnlock(pthread_mutex_t *mutex) = 0;
    virtual int32_t cPthreadCondTimedWait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime) = 0;
    virtual int32_t cPthreadCondWait(pthread_cond_t *cond, pthread_mutex_t *mutex) = 0;
    virtual int32_t cPthreadCondSignal(pthread_cond_t *cond) = 0;
    virtual int32_t cPthreadCondBroadcast(pthread_cond_t *cond) = 0;
    virtual int32_t cPthreadKeyCreate(pthread_key_t *key, void (*destructor)(void*)) = 0;
    virtual int32_t cPthreadKeyDelete(pthread_key_t key) = 0;
    virtual int32_t cPthreadAttrInit(pthread_attr_t *attr) = 0;
    virtual int32_t cPthreadAttrDestroy(pthread_attr_t *attr) = 0;
    virtual int32_t cPthreadSetSpecific(pthread_key_t key, const void *value) = 0;
    virtual pid_t cGetPid() = 0;
    virtual pid_t cGetPPid() = 0;
    
    
    /***
     * Process Scheduling APIs
     */
    virtual int32_t cSchedGetPriorityMax(int32_t policy) = 0;
    virtual int32_t cSchedGetPriorityMin(int32_t policy) = 0;
    virtual int32_t cPthreadAttrSetSchedPolicy(pthread_attr_t *attr, int32_t policy) = 0;
    virtual int32_t cPthreadAttrSetSchedParam(pthread_attr_t *attr, const struct sched_param *param) = 0;
    virtual int32_t cPthreadAttrSetInheritSched(pthread_attr_t *attr, int32_t inheritsched) = 0;
    
    /***
     * Filesystem APIs
     */
    virtual int32_t cAccess(const char *pathname, int32_t mode) = 0;
    virtual int32_t cMkdir(const char *pathname, mode_t mode) = 0;
    virtual int32_t cChmod(const char *pathname, mode_t mode) = 0;
    virtual FILE *cFopen(const char *pathname, const char *mode) = 0;
    virtual int32_t cFclose(FILE *stream) = 0;
    
    /***
     * Time APIs
     */
    virtual int32_t cClockGetTime(clockid_t clockid, struct timespec *tp) = 0;
    virtual int32_t cClockSetTime(clockid_t clockid, const struct timespec *tp) = 0;
    virtual int32_t cUsleep(useconds_t usec) = 0;
    virtual uint32_t cSleep(uint32_t seconds) = 0;
    
    /***
     * String and memory APIs
     */
    virtual void *cMemset(void *s, int32_t c, size_t n) = 0;
    virtual void *cMemcpy(void *dest, const void *src, size_t n) = 0;
    virtual int32_t cStrcmp(const char *s1, const char *s2) = 0;
    virtual char *cStrcpy(char *dst, const char *src) = 0;
    
    /***
     * SYSV Message Queue APIs
     */
    virtual key_t cFtok(const char *pathname, int32_t proj_id) = 0;
    virtual int32_t cMsgGet(key_t key, int32_t msgflg) = 0;
    virtual int32_t cMsgctl(int32_t msqid, int32_t op, struct msqid_ds *buf) = 0;
    virtual ssize_t cMsgrcv(int32_t msqid, void *msgp, size_t msgsz, long msgtyp, int32_t msgflg) = 0;
    virtual int32_t cMsgsnd(int32_t msqid, const void *msgp, size_t msgsz, int32_t msgflg) = 0;
    
    

protected:
    CWrapperIf() = default;
    virtual ~CWrapperIf() = default;
}; // class CWrapperIf

} // namespace INTERNAL
} // namespace ItcPlatform