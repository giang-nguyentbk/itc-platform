#pragma once


#include <atomic>
#include <array>
#include <optional>
#include <functional>
#include <gtest/gtest.h>

namespace ITC
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

template <typename Key, typename Value, size_t NumBuckets, size_t EntriesPerBucket>
class LockFreeHashMap
{
private:
    struct alignas(64) Entry
    {
        std::atomic_bool used{false};
        Key key;
        Value value;
    };

    struct Bucket
    {
        std::array<Entry, EntriesPerBucket> entries;
    };

    std::array<Bucket, NumBuckets> buckets;
    std::hash<Key> hasher;

public:
    LockFreeHashMap() = default;
    ~LockFreeHashMap() = default;

    LockFreeHashMap(const LockFreeHashMap& other)
    {
        for (size_t i = 0; i < NumBuckets; ++i)
        {
            for (size_t j = 0; j < EntriesPerBucket; ++j)
            {
                const auto& src = other.buckets[i].entries[j];
                auto& dst = buckets[i].entries[j];
                bool active = src.used.load(std::memory_order_acquire);
                dst.used.store(active, std::memory_order_release);
                if (active)
                {
                    dst.key = src.key;
                    dst.value = src.value;
                }
            }
        }
    }

    LockFreeHashMap& operator=(const LockFreeHashMap& other)
    {
        if (this != &other) {
            for (size_t i = 0; i < NumBuckets; ++i)
            {
                for (size_t j = 0; j < EntriesPerBucket; ++j)
                {
                    const auto& src = other.buckets[i].entries[j];
                    auto& dst = buckets[i].entries[j];
                    bool active = src.used.load(std::memory_order_acquire);
                    dst.used.store(active, std::memory_order_release);
                    if (active)
                    {
                        dst.key = src.key;
                        dst.value = src.value;
                    }
                }
            }
        }
        return *this;
    }

    // Move constructor
    LockFreeHashMap(LockFreeHashMap&& other) noexcept
    {
        for (size_t i = 0; i < NumBuckets; ++i)
        {
            for (size_t j = 0; j < EntriesPerBucket; ++j)
            {
                auto& src = other.buckets[i].entries[j];
                auto& dst = buckets[i].entries[j];
                bool active = src.used.load(std::memory_order_acquire);
                dst.used.store(active, std::memory_order_release);
                if (active)
                {
                    dst.key = std::move(src.key);
                    dst.value = std::move(src.value);
                }
                src.used.store(false, std::memory_order_release);
            }
        }
    }

    // Move assignment
    LockFreeHashMap& operator=(LockFreeHashMap&& other) noexcept
    {
        if (this != &other)
        {
            for (size_t i = 0; i < NumBuckets; ++i)
            {
                for (size_t j = 0; j < EntriesPerBucket; ++j)
                {
                    auto& src = other.buckets[i].entries[j];
                    auto& dst = buckets[i].entries[j];
                    bool active = src.used.load(std::memory_order_acquire);
                    dst.used.store(active, std::memory_order_release);
                    if (active)
                    {
                        dst.key = std::move(src.key);
                        dst.value = std::move(src.value);
                    }
                    src.used.store(false, std::memory_order_release);
                }
            }
        }
        return *this;
    }

    bool insert(const Key& key, const Value& value)
    {
        size_t index = hasher(key) % NumBuckets;
        auto& bucket = buckets[index];

        for (auto& entry : bucket.entries)
        {
            bool expected = false;
            if (entry.used.compare_exchange_strong(expected, true, std::memory_order_acq_rel))
            {
                entry.key = key;
                entry.value = value;
                return true;
            }
        }
        return false; // bucket full
    }

    std::optional<Value> find(const Key& key)
    {
        size_t index = hasher(key) % NumBuckets;
        auto& bucket = buckets[index];

        for (const auto& entry : bucket.entries)
        {
            if (entry.used.load(std::memory_order_acquire) && entry.key == key)
            {
                return entry.value;
            }
        }
        return std::nullopt;
    }

    bool remove(const Key& key)
    {
        size_t index = hasher(key) % NumBuckets;
        auto& bucket = buckets[index];

        for (auto& entry : bucket.entries)
        {
            if (entry.used.load(std::memory_order_acquire) && entry.key == key)
            {
                entry.used.store(false, std::memory_order_release);
                return true;
            }
        }
        return false;
    }
    
    void clear()
    {
        for (auto& bucket : buckets)
        {
            for (auto& entry : bucket.entries)
            {
                entry.used.store(false, std::memory_order_release);
            }
        }
    }
    
    friend class LockFreeHashMapTest;
	FRIEND_TEST(LockFreeHashMapTest, test1);
	FRIEND_TEST(LockFreeHashMapTest, test2);
	FRIEND_TEST(LockFreeHashMapTest, test3);
};

} // namespace INTERNAL
} // namespace ITC