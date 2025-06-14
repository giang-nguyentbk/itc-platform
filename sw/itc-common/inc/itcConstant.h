#pragma once

#include <cstdint>
#include <cmath>
#include <limits>

#include "itcAdminMessage.h"
#include "itc.h"

namespace ITC
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

/***
 * TODO: move this macro into common-utils project
 */
#ifdef __cplusplus
#define LIKELY [[likely]] // C++20
#define UNLIKELY [[unlikely]] // C++20
#define DEPRECATED(reason) [[deprecated(reason)]] // C++14
#define MAYBE_UNUSED [[maybe_unused]] // C++17
#define NODISCARD(reason) [[nodiscard(reason)]] // C++20
#endif

#define C_LIKELY_WITH_PROBABILITY(cond, ret, proba) \
	__builtin_expect_with_probability(cond, ret, proba) /* proba = 0.0 -> 1.0 */
// #define C_LIKELY(cond, ret) __builtin_expect(cond, ret) /* proba is 90% by default */
#define C_LIKELY(x)      	__builtin_expect(!!(x), 1)
#define C_UNLIKELY(x)    	__builtin_expect(!!(x), 0)
#define ALWAYS_INLINE   	__attribute__((always_inline)) inline

#define ITC_FLAG_I_AM_ITC_SERVER                                    (uint32_t)(0x00000001)
#define ITC_MASK_UNIT_ID                                            (uint32_t)(0x0000FFFF)
#define ITC_MASK_REGION_ID                                          (uint32_t)(0xFFF00000)
#define ITC_REGION_ID_SHIFT                                         (uint32_t)(20) /* Right shift by 20 bits to get region ID */
#define ITC_MAX_SOCKET_RX_BUFFER_SIZE                               (uint32_t)(1024)
#define ITC_MAX_SUPPORTED_REGIONS                                   (uint32_t)(255)
#define ITC_PATH_ITC_DIRECTORY_POSITION                             (size_t)(2) /* 0th is '/', 1st is '/tmp', so 2nd is '/tmp/itc' */
#define ITC_PATH_ITC_SERVER_SOCKET                                  "/tmp/itc/itc-server/itc-server"
// #define ITC_PATH_ITC_SERVER_PROGRAM                                 "/usr/local/bin/itc-server"
#define ITC_PATH_ITC_SERVER_PROGRAM                                 "/home/etrugia/workspace/test/daemon"

#define ITC_NR_INTERNAL_USED_MAILBOXES                              (size_t)(1)

#define SINGLETON_DECLARATION(ClassName) \
    static std::shared_ptr<ClassName> m_instance; \
	static std::mutex m_singletonMutex;

#define SINGLETON_DEFINITION(ClassName) \
    std::shared_ptr<ClassName> ClassName::m_instance = nullptr; \
    std::mutex ClassName::m_singletonMutex; \
    std::weak_ptr<ClassName> ClassName::getInstance() \
    { \
        std::scoped_lock<std::mutex> lock(m_singletonMutex); \
        if (!m_instance) \
        { \
            m_instance.reset(new ClassName); \
        } \
        return m_instance; \
    }
    
#define SINGLETON_IF_DEFINITION(ClassIfName, ClassName) \
    std::weak_ptr<ClassIfName> ClassIfName::getInstance() \
    { \
        return ClassName::getInstance(); \
    }

class ItcMath
{
public:
    static bool areAlmostEqual(double x, double y) {
        return std::fabs(x - y) < std::numeric_limits<double>::epsilon();
    }

    static bool isGreaterThan(double x, double y) {
        return (x - y) > std::numeric_limits<double>::epsilon();
    }

    static bool isLessThan(double x, double y) {
        return (y - x) > std::numeric_limits<double>::epsilon();
    }
};

} // namespace INTERNAL
} // namespace ITC