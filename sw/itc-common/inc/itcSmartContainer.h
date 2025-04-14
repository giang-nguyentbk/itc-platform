#pragma once

#include <vector>
#include <queue>
#include <forward_list>
#include <unordered_map>
#include <string>
#include <functional>
#include <shared_mutex>
#include <mutex>
#include <optional>

#include "itcConstant.h"
#include <gtest/gtest.h>

namespace ITC
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

/***
 * SmartContainer is used for data types that:
 * + Able to fast lookup data via a string key.
 * + Able to random access.
 * + Able to insert/remove elements in the middle efficiently.
 * + Able to save memory as much as possible (by reusing virtually removed slot).
 * + Already thread-safe, no need to create locks for SmartContainer,
 * but locks for elements are still mandantory.
 */
template<typename T>
class SmartContainer
{
public:
	using KeyExtractor = std::function<std::string(const T &)>;
	using DataDestructor = std::function<void(T &)>;
	using Index = std::function<void(T &)>;
	struct Callback
	{
		KeyExtractor keyExtractor;
		DataDestructor dataDestructor;
	};
	
	SmartContainer(const Callback &callback)
		: m_callback(callback)
	{
		m_slotMetadata.reserve(16);
	}
	
	~SmartContainer()
	{
		clear();
	}
	
	SmartContainer(const SmartContainer &other) = default;
	SmartContainer &operator=(const SmartContainer &other) = default;
	SmartContainer(SmartContainer &&other) noexcept = default;
	SmartContainer &operator=(SmartContainer &&other) noexcept = default;
	
    std::optional<size_t> emplace(T &&data)
	{
		std::unique_lock emplaceLock(m_insertionRemovalIsOnGoingMutex);
		/* Check if data already exists */
		auto key = m_callback.keyExtractor(data);
		auto found = m_activeSlots.find(key);
		if(found != m_activeSlots.cend())
		{
			/* Already exist */
			// std::cout << "ETRUGIA: insert index = " << index << ", already exists" << std::endl;
			return std::nullopt;
		}
		
		/* Try to take an inactive slots from queue first, if queue empty,
		emplace_after data to forward_list and emplace_back metadata to vector. */
		size_t index {0xFFFFFFFF};
		if(!m_inactiveSlots.empty())
		{
			index = m_inactiveSlots.front();
			m_inactiveSlots.pop();
			auto &metadata = m_slotMetadata.at(index);
			auto slot = metadata.slot;
			*slot = std::move(data);
		} else
		{
			index = m_slotMetadata.size();
			typename std::forward_list<T>::iterator tail;
			if(m_slotMetadata.empty())
			{
				tail = m_slots.before_begin();
			} else
			{
				tail = m_slotMetadata.back().slot;
			}
			auto slot = m_slots.emplace_after(tail, std::move(data));
			m_slotMetadata.emplace_back(slot);
		}
		
		/* Add inserted item to the hash map. */
		m_activeSlots.emplace(std::make_pair(key, index));
		
		/* Update isActive flag. */
		std::unique_lock metadataLock(*m_slotMetadata.at(index).metadataMutex);
		m_slotMetadata.at(index).isActive = true;
		// std::cout << "ETRUGIA: insert index = " << index << ", added" << std::endl;
		return index;
	}
	
	void remove(size_t index)
	{
		// std::cout << "ETRUGIA: remove1 index = " << index << std::endl;
		std::unique_lock removeLock(m_insertionRemovalIsOnGoingMutex);
		if(index >= m_slotMetadata.size())
		{
			/* Invalid access: out of range. */
			// std::cout << "ETRUGIA: remove1 index = " << index << ", out of range" << std::endl;
			return;
		}
		
		auto &metadata = m_slotMetadata.at(index);
		
		{
			std::unique_lock metadataLock(*metadata.metadataMutex);
			if(!metadata.isActive)
			{
				/* An already inactive slot. */
				// std::cout << "ETRUGIA: remove1 index = " << index << ", inactive" << std::endl;
				return;
			}
			metadata.isActive = false;
		}
		
		auto key = m_callback.keyExtractor(*metadata.slot);
		m_activeSlots.erase(key);
		m_inactiveSlots.push(index);
		
		m_callback.dataDestructor(*metadata.slot);
	}
	
	void remove(const std::string &key)
	{
		std::unique_lock removeLock(m_insertionRemovalIsOnGoingMutex);
		// std::cout << "ETRUGIA: remove2 key = " << key;
		auto found = m_activeSlots.find(key);
		if(found == m_activeSlots.cend())
		{
			/* Invalid access: not found. */
			// std::cout << ", key not found" << std::endl;
			return;
		}
		
		// std::cout << ", index = " << found->second << std::endl;
		removeLock.unlock();
		remove(found->second);
	}
	
	std::optional<std::reference_wrapper<T>> at(size_t index)
	{
		std::shared_lock atLock(m_insertionRemovalIsOnGoingMutex);
		if(index >= m_slotMetadata.size())
		{
			/* Invalid access: out of range. */
			return std::nullopt;
		}
		
		auto &metadata = m_slotMetadata.at(index);
		std::unique_lock metadataLock(*metadata.metadataMutex);
		if(!metadata.isActive)
		{
			/* Invalid access: inactive slot. */
			return std::nullopt;
		}
		
		return *(metadata.slot);
	}
	
	std::optional<std::reference_wrapper<T>> at(const std::string &key)
	{
		std::shared_lock atLock(m_insertionRemovalIsOnGoingMutex);
		auto found = m_activeSlots.find(key);
		if(found == m_activeSlots.cend())
		{
			/* Invalid access: not found. */
			return std::nullopt;
		}
		
		atLock.unlock();
		return at(found->second);
	}
	
	std::optional<size_t> getIndex(const std::string &key)
	{
		std::shared_lock getIndexLock(m_insertionRemovalIsOnGoingMutex);
		auto found = m_activeSlots.find(key);
		if(found == m_activeSlots.cend())
		{
			return std::nullopt;
		}
		return found->second;
	}
	
	T *data(size_t index)
	{
		std::shared_lock dataLock(m_insertionRemovalIsOnGoingMutex);
		if(index >= m_slotMetadata.size())
		{
			/* Invalid access: out of range. */
			return nullptr;
		}
		
		auto &metadata = m_slotMetadata.at(index);
		std::unique_lock metadataLock(*metadata.metadataMutex);
		if(!metadata.isActive)
		{
			/* Invalid access: inactive slot. */
			return nullptr;
		}
		
		return &(*(metadata.slot));
	}
	
	T *data(const std::string &key)
	{
		std::shared_lock dataLock(m_insertionRemovalIsOnGoingMutex);
		auto found = m_activeSlots.find(key);
		if(found == m_activeSlots.cend())
		{
			/* Invalid access: not found. */
			return nullptr;
		}
		
		dataLock.unlock();
		return data(found->second);
	}
	
	bool exist(size_t index)
	{
		std::shared_lock existLock(m_insertionRemovalIsOnGoingMutex);
		if(index >= m_slotMetadata.size())
		{
			/* Invalid access: out of range. */
			return false;
		}
		
		auto &metadata = m_slotMetadata.at(index);
		std::unique_lock metadataLock(*metadata.metadataMutex);
		if(!metadata.isActive)
		{
			/* Invalid access: inactive slot. */
			return false;
		}
		
		return true;
	}
	
	bool exist(const std::string &key)
	{
		std::shared_lock existLock(m_insertionRemovalIsOnGoingMutex);
		auto found = m_activeSlots.find(key);
		if(found == m_activeSlots.cend())
		{
			/* Invalid access: not found. */
			return false;
		}
		
		existLock.unlock();
		return exist(found->second);
	}
	
	size_t size()
	{
		std::shared_lock sizeLock(m_insertionRemovalIsOnGoingMutex);
		return m_slotMetadata.size() - m_inactiveSlots.size();
	}
	
	size_t capacity()
	{
		std::shared_lock capacityLock(m_insertionRemovalIsOnGoingMutex);
		return m_slotMetadata.size();
	}
	
	void clear()
	{
		std::unique_lock clearLock(m_insertionRemovalIsOnGoingMutex);
		while(!m_inactiveSlots.empty())
		{
			m_inactiveSlots.pop();
		}
		m_activeSlots.clear();
		m_slotMetadata.clear();
		m_slots.clear();
	}
	
private:
	struct MetaData
	{
		std::unique_ptr<std::mutex> metadataMutex;
		typename std::forward_list<T>::iterator slot;
		bool isActive {false};
		
		MetaData(typename std::forward_list<T>::iterator iSlot = typename std::forward_list<T>::iterator{},
             bool iIsActive = false)
			: slot(iSlot),
			  isActive(iIsActive)
    	{
			metadataMutex = std::make_unique<std::mutex>();
		}
		MetaData() = default;
		~MetaData() = default;
		
		MetaData(const MetaData& other)			= delete;
		MetaData& operator=(const MetaData&)    = delete;
		MetaData(MetaData&&)                    = default;
		MetaData& operator=(MetaData&&)         = default;
	};
	
private:
	Callback m_callback;
	std::shared_mutex m_insertionRemovalIsOnGoingMutex;
	std::forward_list<T> m_slots;
	std::vector<MetaData> m_slotMetadata;
	std::unordered_map<std::string, size_t> m_activeSlots;
	std::queue<size_t> m_inactiveSlots;
	
	friend class SmartContainerTest;
	FRIEND_TEST(SmartContainerTest, test1);
	FRIEND_TEST(SmartContainerTest, test2);
	FRIEND_TEST(SmartContainerTest, test3);
	FRIEND_TEST(SmartContainerTest, test4);
	FRIEND_TEST(SmartContainerTest, test5);
	FRIEND_TEST(SmartContainerTest, test6);
	FRIEND_TEST(SmartContainerTest, test7);
	FRIEND_TEST(SmartContainerTest, test8);
	FRIEND_TEST(SmartContainerTest, test9);
	FRIEND_TEST(SmartContainerTest, stressTest1);
	FRIEND_TEST(SmartContainerTest, stressTest2);
	FRIEND_TEST(SmartContainerTest, stressTest3);
	FRIEND_TEST(SmartContainerTest, stressTest4);
	FRIEND_TEST(SmartContainerTest, stressTest5);
};

} // namespace INTERNAL
} // namespace ITC