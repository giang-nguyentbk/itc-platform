#pragma once

#include <atomic>
#include <cstdint>
#include <unordered_map>

#include "itcLockFreeQueue.h"

namespace ITC
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

#define CACHE_LINE_BYTES (uint8_t)(64)
#define GET_ALIGNED_ENTRY(i) \
    reinterpret_cast<RawPtr>(reinterpret_cast<uint8_t *>(m_rawData) + (i * CACHE_LINE_BYTES))

template<typename T, uint32_t SIZE, T NIL>
class ConcurrentArray
{
public:
    using RawPtr = T *;
    
    ConcurrentArray()
    {
        
        static_assert(sizeof(T) <= 64, "Otherwise, cannot apply lock-free for this type T!");
        m_rawEntries = new uint8_t[SIZE * CACHE_LINE_BYTES];
        for(uint32_t i = 0; i < SIZE; ++i)
        {
            RawPtr entry = GET_ALIGNED_ENTRY(i);
            entry->T();
            m_inactiveEntries.tryPush(entry);
        }
    }
    
private:
    uint8_t *m_rawEntries {nullptr};
    LockFreeQueue<RawPtr, SIZE, nullptr, true, true, false, false> m_inactiveEntries;
    
    std::atomic_bool m_activeEntriesLock {false};
    std::unordered_map<std::string, RawPtr> m_activeEntries;
};

} // namespace INTERNAL
} // namespace ITC