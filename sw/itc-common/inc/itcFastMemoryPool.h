#pragma once

#include <cstdint>
#include "itcLockFreeQueue.h"

namespace ITC
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

template<typename T>
class FastMemoryPool
{
    using RawPtr = uint8_t *;
    

private:
    LockFreeQueue<RawPtr, 256, nullptr, true, true, false, false> m_pool_96;
    LockFreeQueue<RawPtr, 256, nullptr, true, true, false, false> m_pool_96;
    LockFreeQueue<RawPtr, 256, nullptr, true, true, false, false> m_pool_96;
    LockFreeQueue<RawPtr, 256, nullptr, true, true, false, false> m_pool_96;
    
}; // class FastMemoryPool

} // namespace INTERNAL
} // namespace ITC