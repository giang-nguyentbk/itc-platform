#pragma once

#include <atomic>
#include <cstdint>
#include <unordered_map>
#include <string>
#include <functional>

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
    reinterpret_cast<RawPtr>(reinterpret_cast<uint8_t *>(m_rawEntries) + (i * CACHE_LINE_BYTES))


template<typename T, uint32_t SIZE>
class ConcurrentContainer
{
public:
    using RawPtr = T *;
    
    ConcurrentContainer(std::function<void(T *, uint32_t)> initializer)
    {
        
        static_assert(sizeof(T) <= 64, "Otherwise, cannot apply lock-free for this type T!");
        m_rawEntries = new uint8_t[SIZE * CACHE_LINE_BYTES];
        for(uint32_t i = 0; i < SIZE; ++i)
        {
            RawPtr entry = GET_ALIGNED_ENTRY(i);
            entry = new (reinterpret_cast<uint8_t *>(entry)) T();
            initializer(entry, i);
            m_inactiveEntries.tryPush(entry);
        }
        
    }
    
    ~ConcurrentContainer()
    {
        delete[] m_rawEntries;
        while(!m_inactiveEntries.empty())
        {
            m_inactiveEntries.pop();
        }
        m_activeEntries.clear();
    }
    
    RawPtr at(uint32_t index)
    {
        return GET_ALIGNED_ENTRY(index);
    }
    
    bool addEntryToHashMap(const std::string &key, RawPtr entry)
    {
        std::unique_lock lock(m_activeEntriesLock);
        auto found = m_activeEntries.find(key);
        if(found != m_activeEntries.cend()) UNLIKELY
        {
            return false;
        }
        m_activeEntries.emplace(key, entry);
        return true;
    }
    
    bool removeEntryFromHashMap(const std::string &key)
    {
        std::unique_lock lock(m_activeEntriesLock);
        auto found = m_activeEntries.find(key);
        if(found == m_activeEntries.cend()) UNLIKELY
        {
            return false;
        }
        m_activeEntries.erase(key);
        return true;
    }
    
    RawPtr lookUpFromHashMap(const std::string &key)
    {
        std::unique_lock lock(m_activeEntriesLock);
        auto found = m_activeEntries.find(key);
        if(found != m_activeEntries.cend()) UNLIKELY
        {
            return nullptr;
        }
        return found->second;
    }
    
    bool tryPushIntoQueue(RawPtr entry)
    {
        return m_inactiveEntries.tryPush(entry);
    }
    
    RawPtr tryPopFromQueue()
    {
        return m_inactiveEntries.pop();
    }
    
    uint32_t size()
    {
        return SIZE - m_inactiveEntries.size();
    }
        
private:
    uint8_t *m_rawEntries {nullptr};
    LockFreeQueue<RawPtr, SIZE, nullptr, MINIMIZE_CONTENTION, MAXIMIZE_THROUGHPUT, !IS_TOTAL_ORDER, !IS_SPSC> m_inactiveEntries;
    
    std::mutex m_activeEntriesLock;
    std::unordered_map<std::string, RawPtr> m_activeEntries;
};

} // namespace INTERNAL
} // namespace ITC