#pragma once

#include "itcCWrapperIf.h"
#include "itcConstant.h"

#include <mutex>
#include <memory>

namespace ITC
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

class CWrapper : public CWrapperIf
{
public:
    static std::weak_ptr<CWrapper> getInstance();
	virtual ~CWrapper() = default;

    
    // First prevent copy/move construtors/assignments
	CWrapper(const CWrapper&)               = delete;
	CWrapper(CWrapper&&)                    = delete;
	CWrapper& operator=(const CWrapper&)    = delete;
	CWrapper& operator=(CWrapper&&)         = delete;

    /***
     * Network APIs
     */
    int32_t cSocket(int32_t domain, int32_t type, int32_t protocol) override;
    int32_t cConnect(int32_t sockfd, const struct sockaddr *addr, socklen_t addrlen) override;
    ssize_t cSend(int32_t sockfd, const void *buf, size_t size, int32_t flags) override;
    ssize_t cRecv(int32_t sockfd, void *buf, size_t size, int32_t flags) override;
    int32_t cClose(int32_t fd) override;
    
    /***
     * Threading APIs
     */
    int32_t cPthreadCondAttrInit(pthread_condattr_t *attr) override;
    int32_t cPthreadMutexAttrInit(pthread_mutexattr_t *attr) override;
    int32_t cPthreadMutexAttrSetType(pthread_mutexattr_t *attr, int32_t type) override;
    int32_t cPthreadCondAttrSetClock(pthread_condattr_t *attr, clockid_t clock_id) override;
    int32_t cPthreadCondInit(pthread_cond_t *cond, const pthread_condattr_t *attr) override;
    int32_t cPthreadMutexInit(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr) override;
    int32_t cPthreadCondAttrDestroy(pthread_condattr_t *attr) override;
    int32_t cPthreadMutexAttrDestroy(pthread_mutexattr_t *attr) override;
    int32_t cPthreadCondDestroy(pthread_cond_t *cond) override;
    int32_t cPthreadMutexDestroy(pthread_mutex_t *mutex) override;
    int32_t cPthreadCreate(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg) override;
    int32_t cPthreadCancel(pthread_t thread) override;
    int32_t cPthreadJoin(pthread_t thread, void **retval) override;
    int32_t cPthreadDetach(pthread_t thread) override;
    int32_t cPthreadMutexLock(pthread_mutex_t *mutex) override;
    int32_t cPthreadMutexUnlock(pthread_mutex_t *mutex) override;
    int32_t cPthreadCondTimedWait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime) override;
    int32_t cPthreadCondWait(pthread_cond_t *cond, pthread_mutex_t *mutex) override;
    int32_t cPthreadCondSignal(pthread_cond_t *cond) override;
    int32_t cPthreadCondBroadcast(pthread_cond_t *cond) override;
    int32_t cPthreadKeyCreate(pthread_key_t *key, void (*destructor)(void*)) override;
    int32_t cPthreadKeyDelete(pthread_key_t key) override;
    int32_t cPthreadAttrInit(pthread_attr_t *attr) override;
    int32_t cPthreadAttrDestroy(pthread_attr_t *attr) override;
    int32_t cPthreadSetSpecific(pthread_key_t key, const void *value) override;
    int32_t cPthreadSetCancelState(int32_t state, int32_t *oldstate) override;
    pid_t cGetPid() override;
    pid_t cGetPPid() override;
    
    
    /***
     * Process Scheduling APIs
     */
    int32_t cSchedGetPriorityMax(int32_t policy) override;
    int32_t cSchedGetPriorityMin(int32_t policy) override;
    int32_t cPthreadAttrSetSchedPolicy(pthread_attr_t *attr, int32_t policy) override;
    int32_t cPthreadAttrSetSchedParam(pthread_attr_t *attr, const struct sched_param *param) override;
    int32_t cPthreadAttrSetInheritSched(pthread_attr_t *attr, int inheritsched) override;
    
    /***
     * Filesystem APIs
     */
    int32_t cAccess(const char *pathname, int32_t mode) override;
    int32_t cMkdir(const char *pathname, mode_t mode) override;
    int32_t cChmod(const char *pathname, mode_t mode) override;
    FILE *cFopen(const char *pathname, const char *mode) override;
    int32_t cFclose(FILE *stream) override;
    int32_t cEventFd(uint32_t initval, int32_t flags) override;
    ssize_t cRead(int32_t fd, void *buf, size_t count) override;
    ssize_t cWrite(int32_t fd, const void *buf, size_t count) override;
    
    /***
     * Time APIs
     */
    int32_t cClockGetTime(clockid_t clockid, struct timespec *tp) override;
    int32_t cClockSetTime(clockid_t clockid, const struct timespec *tp) override;
    int32_t cUsleep(useconds_t usec) override;
    uint32_t cSleep(uint32_t seconds) override;
    
    /***
     * String and memory APIs
     */
    void *cMemset(void *s, int32_t c, size_t n) override;
    void *cMemcpy(void *dest, const void *src, size_t n) override;
    int32_t cStrcmp(const char *s1, const char *s2) override;
    char *cStrcpy(char *dst, const char *src) override;
    size_t cStrlen(const char *s) override;
    
    /***
     * SYSV Message Queue APIs
     */
    key_t cFtok(const char *pathname, int32_t proj_id) override;
    int32_t cMsgGet(key_t key, int msgflg) override;
    int32_t cMsgctl(int32_t msqid, int32_t op, struct msqid_ds *buf) override;
    ssize_t cMsgrcv(int32_t msqid, void *msgp, size_t msgsz, long msgtyp, int32_t msgflg) override;
    int32_t cMsgsnd(int32_t msqid, const void *msgp, size_t msgsz, int32_t msgflg) override;

private:
    CWrapper() = default;

private:
    SINGLETON_DECLARATION(CWrapper)

}; // class CWrapper

} // namespace INTERNAL
} // namespace ITC