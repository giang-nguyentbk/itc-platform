#pragma once

#include <vector>
#include <array>
#include <string>
#include <functional>
#include <shared_mutex>
#include <mutex>
#include <limits>

#include "itcLockFreeQueue.h"
#include "itcLockFreeHashMap.h"
#include <gtest/gtest.h>

namespace ITC
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

constexpr size_t INVALID_INDEX = std::numeric_limits<size_t>::max();

template<class T>
constexpr T NIL() noexcept
{
    return T();
}

/***
 * ConcurrentContainer is used for data types that:
 * + Able to fast lookup data via a string key.
 * + Able to random access.
 * + Able to insert/remove elements in the middle efficiently.
 * + Able to save memory as much as possible (by reusing virtually removed slot).
 * + Already thread-safe, no need to create locks for ConcurrentContainer,
 * but locks for elements are still mandantory.
 */
template<typename T, uint32_t SIZE = 128>
class ConcurrentContainer
{
public:
	explicit ConcurrentContainer(const T &NIL)
		: m_NIL(NIL)
	{
		for(size_t i = 0; i < SIZE; ++i)
		{
			m_entries.at(i).data = m_NIL;
			m_inactiveEntries.tryPush(i);
		}
	}
	
	~ConcurrentContainer()
	{
		clear();
	}
	
	ConcurrentContainer(const ConcurrentContainer &other) = default;
	ConcurrentContainer &operator=(const ConcurrentContainer &other) = default;
	ConcurrentContainer(ConcurrentContainer &&other) noexcept = default;
	ConcurrentContainer &operator=(ConcurrentContainer &&other) noexcept = default;
	
    size_t insert(const std::string &key, T &&data)
	{
		while(m_isInUse.test_and_set(MEMORY_ORDER_ACQUIRE))
		{
			yieldProcessor();
		}
		
		size_t index = INVALID_INDEX;
		/* Check if data already exists */
		auto found = m_activeEntries.find(key);
		if(found.has_value())
		{
			/* Already exist */
			// std::cout << "ETRUGIA: insert index = " << index << ", already exists" << std::endl;
			m_isInUse.clear(MEMORY_ORDER_RELEASE);
			return index;
		}
		
		m_inactiveEntries.tryPop(index);
		m_activeEntries.insert(key, index);
		auto &entry = m_entries.at(index);
		entry.data = std::move(data);
		entry.ready.store(true, MEMORY_ORDER_RELAXED);
		m_isInUse.clear(MEMORY_ORDER_RELEASE);
		return index;
	}
	
	bool recycle(const std::string &key)
	{
		while(m_isInUse.test_and_set(MEMORY_ORDER_ACQUIRE))
		{
			yieldProcessor();
		}
		
		// std::cout << "ETRUGIA: remove2 key = " << key;
		auto found = m_activeEntries.find(key);
		if(!found.has_value())
		{
			/* Invalid access: not found. */
			// std::cout << ", key not found" << std::endl;
			m_isInUse.clear(MEMORY_ORDER_RELEASE);
			return false;
		}
		
		// std::cout << ", index = " << found->second << std::endl;
		auto &entry = m_entries.at(found.value());
		entry.ready.store(false, MEMORY_ORDER_RELAXED);
		
		m_activeEntries.remove(key);
		m_inactiveEntries.tryPush(found.value());
		entry.data.~T();
		m_isInUse.clear(MEMORY_ORDER_RELEASE);
		return true;
	}
	
	T &at(size_t index)
	{
		while(m_isInUse.test_and_set(MEMORY_ORDER_ACQUIRE))
		{
			yieldProcessor();
		}
		
		auto &entry = m_entries.at(index);
		if(!entry.ready.load(MEMORY_ORDER_RELAXED))
		{
			m_isInUse.clear(MEMORY_ORDER_RELEASE);
			return m_NIL;
		}
		
		m_isInUse.clear(MEMORY_ORDER_RELEASE);
		return entry.data;
	}
	
	T &at(const std::string &key)
	{
		while(m_isInUse.test_and_set(MEMORY_ORDER_ACQUIRE))
		{
			yieldProcessor();
		}
		
		auto found = m_activeEntries.find(key);
		if(!found.has_value())
		{
			/* Invalid access: not found. */
			// std::cout << ", key not found" << std::endl;
			m_isInUse.clear(MEMORY_ORDER_RELEASE);
			return m_NIL;
		}
		
		// std::cout << ", index = " << found->second << std::endl;
		m_isInUse.clear(MEMORY_ORDER_RELEASE);
		return m_entries.at(found.value()).data;
	}
	
	size_t getIndex(const std::string &key)
	{
		while(m_isInUse.test_and_set(MEMORY_ORDER_ACQUIRE))
		{
			yieldProcessor();
		}
		
		auto found = m_activeEntries.find(key);
		if(!found.has_value())
		{
			/* Invalid access: not found. */
			// std::cout << ", key not found" << std::endl;
			m_isInUse.clear(MEMORY_ORDER_RELEASE);
			return INVALID_INDEX;
		}
		
		m_isInUse.clear(MEMORY_ORDER_RELEASE);
		return found.value();
	}
	
	bool exist(size_t index)
	{
		while(m_isInUse.test_and_set(MEMORY_ORDER_ACQUIRE))
		{
			yieldProcessor();
		}
		
		auto &entry = m_entries.at(index);
		if(!entry.ready.load(MEMORY_ORDER_RELAXED))
		{
			m_isInUse.clear(MEMORY_ORDER_RELEASE);
			return false;
		}
		
		m_isInUse.clear(MEMORY_ORDER_RELEASE);
		return true;
	}
	
	bool exist(const std::string &key)
	{
		while(m_isInUse.test_and_set(MEMORY_ORDER_ACQUIRE))
		{
			yieldProcessor();
		}
		
		auto found = m_activeEntries.find(key);
		if(!found.has_value())
		{
			/* Invalid access: not found. */
			// std::cout << ", key not found" << std::endl;
			m_isInUse.clear(MEMORY_ORDER_RELEASE);
			return false;
		}
		
		// std::cout << ", index = " << found->second << std::endl;
		m_isInUse.clear(MEMORY_ORDER_RELEASE);
		return true;
	}
	
	size_t size()
	{
		return SIZE - m_inactiveEntries.size();
	}
	
	void clear()
	{
		while(m_isInUse.test_and_set(MEMORY_ORDER_ACQUIRE))
		{
			yieldProcessor();
		}
		
		[[maybe_unused]] size_t dummy {0};
		while(!m_inactiveEntries.empty())
		{
			m_inactiveEntries.tryPop(dummy);
		}
		
		m_activeEntries.clear();
		m_isInUse.clear(MEMORY_ORDER_RELEASE);
	}
	
private:
	struct alignas(64) Entry
	{
		std::atomic_bool ready {false};
		T data;
		
		Entry(T &&d)
    	{
			data = std::move(d);
		}
		Entry() = default;
		~Entry() = default;
		
		Entry(const Entry&)					= delete;
		Entry& operator=(const Entry&)    	= delete;
		Entry(Entry&&)                    	= delete;
		Entry& operator=(Entry&&)         	= delete;
	};
	
private:
	T m_NIL;
	std::atomic_flag m_isInUse {};
	std::array<Entry, SIZE> m_entries;
	LockFreeHashMap<std::string, size_t, SIZE + SIZE/2, 16> m_activeEntries;
	LockFreeQueue<size_t, SIZE, SIZE, false, true, false, false> m_inactiveEntries;
	
	friend class ConcurrentContainerTest;
	FRIEND_TEST(ConcurrentContainerTest, test1);
	FRIEND_TEST(ConcurrentContainerTest, test2);
};

} // namespace INTERNAL
} // namespace ITC