#pragma once

#include <atomic>
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>
#include <gtest/gtest.h>

#include "itcConstant.h"

namespace ITC
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

static auto constexpr MEMORY_ORDER_ACQUIRE = std::memory_order_acquire;
static auto constexpr MEMORY_ORDER_RELEASE = std::memory_order_release;
static auto constexpr MEMORY_ORDER_RELAXED = std::memory_order_relaxed;
static auto constexpr MEMORY_ORDER_SEQ_CONSISTENT = std::memory_order_seq_cst;
static auto constexpr MEMORY_ORDER_ACQUIRE_RELEASE = std::memory_order_acq_rel;

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#include <emmintrin.h>
constexpr int32_t CACHE_LINE_SIZE = 64;
static inline void yieldProcessor() noexcept {
    _mm_pause();
}
#elif defined(__arm__) || defined(__aarch64__) || defined(_M_ARM64)
constexpr int32_t CACHE_LINE_SIZE = 64;
static inline void yieldProcessor() noexcept {
#if (defined(__ARM_ARCH_6K__) || \
     defined(__ARM_ARCH_6Z__) || \
     defined(__ARM_ARCH_6ZK__) || \
     defined(__ARM_ARCH_6T2__) || \
     defined(__ARM_ARCH_7__) || \
     defined(__ARM_ARCH_7A__) || \
     defined(__ARM_ARCH_7R__) || \
     defined(__ARM_ARCH_7M__) || \
     defined(__ARM_ARCH_7S__) || \
     defined(__ARM_ARCH_8A__) || \
     defined(__aarch64__))
    asm volatile ("yield" ::: "memory");
#elif defined(_M_ARM64)
    __yield();
#else
    asm volatile ("nop" ::: "memory");
#endif
}
#else
constexpr int32_t CACHE_LINE_SIZE = 64;
static inline void yieldProcessor() noexcept {}
#endif

template<class T>
constexpr T nil() noexcept
{
#if __cpp_lib_atomic_is_always_lock_free
    static_assert(std::atomic<T>::is_always_lock_free, "Element type T is not atomicable!");
#endif
    return {};
}

/***
 * To avoid False Sharing on a same processor's cache line (normally 64 bytes), we would need to
 * instead of putting array's elements closely to each other in such way that 2 or more consecutive
 * elements may reside on a same cache line (64 bytes), break them out or space them apart,
 * each element only sits on a single cache line.
 * 
 * For example if you have an array of uint8_t:
 * So, normally 64 elements are packed into a same cache line with index from 0 -> 63.
 * If two threads access and modify two different elements but they're causing cache line invalidation
 * which seriously slows down the performance.
 * 
 * So, we will trade off memory footprint a bit with much better performance
 * by remapping emelents at 64n-th positions in a larger array.
 * index = 0 -> Before remapping: at byte 0th -> After remapping: at byte 0th
 * index = 1 -> Before remapping: at byte 1st -> After remapping: at byte 64th
 * index = 2 -> Before remapping: at byte 2nd -> After remapping: at byte 128th
 * index = 3 -> Before remapping: at byte 3rd -> After remapping: at byte 192th
 * ...
 */
template<size_t elementsPerCacheLine> struct GetCacheLineIndexBits { static int32_t constexpr value = 0; };
template<> struct GetCacheLineIndexBits<256> { static int32_t constexpr value = 8; };
template<> struct GetCacheLineIndexBits<128> { static int32_t constexpr value = 7; };
template<> struct GetCacheLineIndexBits< 64> { static int32_t constexpr value = 6; };
template<> struct GetCacheLineIndexBits< 32> { static int32_t constexpr value = 5; };
template<> struct GetCacheLineIndexBits< 16> { static int32_t constexpr value = 4; };
template<> struct GetCacheLineIndexBits<  8> { static int32_t constexpr value = 3; };
template<> struct GetCacheLineIndexBits<  4> { static int32_t constexpr value = 2; };
template<> struct GetCacheLineIndexBits<  2> { static int32_t constexpr value = 1; };

template<bool WILL_MINIMIZE_CONTENTION, uint32_t SIZE, size_t elementsPerCacheLine>
struct GetIndexShuffleBits
{
    static int32_t constexpr bits = GetCacheLineIndexBits<elementsPerCacheLine>::value;
    static uint32_t constexpr minSize = 1u << (bits * 2);
    static int32_t constexpr value = SIZE < minSize ? 0 : bits;
};

template<uint32_t SIZE, size_t elementsPerCacheLine>
struct GetIndexShuffleBits<false, SIZE, elementsPerCacheLine>
{
    static int32_t constexpr value = 0;
};

template<int32_t BITS>
constexpr uint32_t remapIndex(uint32_t index) noexcept
{
    uint32_t constexpr mix_mask{(1u << BITS) - 1};
    uint32_t const mix{(index ^ (index >> BITS)) & mix_mask};
    return index ^ mix ^ (mix << BITS);
}

template<>
constexpr uint32_t remapIndex<0>(uint32_t index) noexcept
{
    return index;
}

template<int32_t BITS, class T>
constexpr T& map(T *elements, uint32_t index) noexcept
{
    return elements[remapIndex<BITS>(index)];
}

template<class T>
constexpr T decrement(T x) noexcept
{
    return x - 1;
}

template<class T>
constexpr T increment(T x) noexcept
{
    return x + 1;
}

template<class T>
constexpr T orEqual(T x, uint32_t u) noexcept
{
    return x | x >> u;
}

template<class T, class... Args>
constexpr T orEqual(T x, uint32_t u, Args... rest) noexcept
{
    return orEqual(orEqual(x, u), rest...);
}

constexpr uint32_t roundUpToPowerOf2(uint32_t a) noexcept
{
    return increment(orEqual(decrement(a), 1, 2, 4, 8, 16));
}

constexpr uint64_t roundUpToPowerOf2(uint64_t a) noexcept
{
    return increment(orEqual(decrement(a), 1, 2, 4, 8, 16, 32));
}



template<typename DerivedClass>
class LockFreeQueueBase
{
protected:
    // Put these on different cache lines to avoid false sharing between readers and writers.
    alignas(CACHE_LINE_SIZE) std::atomic<uint32_t> m_head = {};
    alignas(CACHE_LINE_SIZE) std::atomic<uint32_t> m_tail = {};

    // The special member functions are not thread-safe.
    LockFreeQueueBase() noexcept = default;
    LockFreeQueueBase(const LockFreeQueueBase &other) noexcept
        : m_head(other.m_head.load(MEMORY_ORDER_RELAXED))
        , m_tail(other.m_tail.load(MEMORY_ORDER_RELAXED))
    {}

    LockFreeQueueBase& operator=(const LockFreeQueueBase &other) noexcept
    {
        m_head.store(other.m_head.load(MEMORY_ORDER_RELAXED), MEMORY_ORDER_RELAXED);
        m_tail.store(other.m_tail.load(MEMORY_ORDER_RELAXED), MEMORY_ORDER_RELAXED);
        return *this;
    }

    void swap(LockFreeQueueBase &&other) noexcept
    {
        uint32_t h = m_head.load(MEMORY_ORDER_RELAXED);
        uint32_t t = m_tail.load(MEMORY_ORDER_RELAXED);
        m_head.store(other.m_head.load(MEMORY_ORDER_RELAXED), MEMORY_ORDER_RELAXED);
        m_tail.store(other.m_tail.load(MEMORY_ORDER_RELAXED), MEMORY_ORDER_RELAXED);
        other.m_head.store(h, MEMORY_ORDER_RELAXED);
        other.m_tail.store(t, MEMORY_ORDER_RELAXED);
    }

    template<class T, T NIL>
    static T doPopAtomic(std::atomic<T> &queueElem) noexcept
    {
        if(DerivedClass::m_isSPSC)
        {
            while(true)
            {
                T element = queueElem.load(MEMORY_ORDER_ACQUIRE);
                if(C_LIKELY(element != NIL))
                {
                    queueElem.store(NIL, MEMORY_ORDER_RELAXED);
                    return element;
                }
                if(DerivedClass::m_willMaximizeThroughput)
                {
                    yieldProcessor();
                }
            }
        } else
        {
            while(true)
            {
                T element = queueElem.exchange(NIL, MEMORY_ORDER_ACQUIRE);
                if(C_LIKELY(element != NIL))
                {
                    return element;
                }
                
                do
                {
                    /* Do speculative loads while busy-waiting to avoid broadcasting RFO messages by CPU processors. */
                    yieldProcessor();
                }
                while(DerivedClass::m_willMaximizeThroughput && queueElem.load(MEMORY_ORDER_RELAXED) == NIL);
            }
        }
    }

    template<class T, T NIL>
    static void doPushAtomic(T element, std::atomic<T> &queueElem) noexcept
    {
        assert(element != NIL);
        if(DerivedClass::m_isSPSC)
        {
            while(C_UNLIKELY(queueElem.load(MEMORY_ORDER_RELAXED) != NIL))
            {
                if(DerivedClass::m_willMaximizeThroughput)
                {
                    yieldProcessor();
                }
            }
            queueElem.store(element, MEMORY_ORDER_RELEASE);
        } else
        {
            for(T expected = NIL; C_UNLIKELY(!queueElem.compare_exchange_weak(expected, element, MEMORY_ORDER_RELEASE, MEMORY_ORDER_RELAXED)); expected = NIL)
            {
                do
                {
                    yieldProcessor();
                }
                while(DerivedClass::m_willMaximizeThroughput && queueElem.load(MEMORY_ORDER_RELAXED) != NIL);
            }
        }
    }

public:
    template<class T>
    ALWAYS_INLINE bool tryPush(T &&element) noexcept
    {
        auto head = m_head.load(MEMORY_ORDER_RELAXED);
        if(DerivedClass::m_isSPSC)
        {
            if(static_cast<int32_t>(head - m_tail.load(MEMORY_ORDER_RELAXED)) >= static_cast<int32_t>(static_cast<DerivedClass &>(*this).m_size))
            {
                return false;
            }
            m_head.store(head + 1, MEMORY_ORDER_RELAXED);
        }
        else {
            do {
                if(static_cast<int32_t>(head - m_tail.load(MEMORY_ORDER_RELAXED)) >= static_cast<int32_t>(static_cast<DerivedClass &>(*this).m_size))
                {
                    return false;
                }
            } while(C_UNLIKELY(!m_head.compare_exchange_weak(head, head + 1, MEMORY_ORDER_RELAXED, MEMORY_ORDER_RELAXED)));
        }

        static_cast<DerivedClass &>(*this).doPush(std::forward<T>(element), head);
        return true;
    }

    template<class T>
    ALWAYS_INLINE bool tryPop(T& element) noexcept
    {
        auto tail = m_tail.load(MEMORY_ORDER_RELAXED);
        if(DerivedClass::m_isSPSC)
        {
            if(static_cast<int32_t>(m_head.load(MEMORY_ORDER_RELAXED) - tail) <= 0)
            {
                return false;
            }
            m_tail.store(tail + 1, MEMORY_ORDER_RELAXED);
        } else
        {
            do
            {
                if(static_cast<int32_t>(m_head.load(MEMORY_ORDER_RELAXED) - tail) <= 0)
                {
                    return false;
                }
            } while(C_UNLIKELY(!m_tail.compare_exchange_weak(tail, tail + 1, MEMORY_ORDER_RELAXED, MEMORY_ORDER_RELAXED)));
        }

        element = static_cast<DerivedClass &>(*this).doPop(tail);
        return true;
    }

    template<class T>
    ALWAYS_INLINE void push(T &&element) noexcept
    {
        uint32_t head;
        if(DerivedClass::m_isSPSC)
        {
            head = m_head.load(MEMORY_ORDER_RELAXED);
            m_head.store(head + 1, MEMORY_ORDER_RELAXED);
        } else
        {
            constexpr auto memoryOrder = DerivedClass::m_isTotalOrder ? std::memory_order_seq_cst : std::memory_order_relaxed;
            head = m_head.fetch_add(1, memoryOrder);
        }
        static_cast<DerivedClass &>(*this).doPush(std::forward<T>(element), head);
    }

    ALWAYS_INLINE auto pop() noexcept
    {
        uint32_t tail;
        if(DerivedClass::m_isSPSC)
        {
            tail = m_tail.load(MEMORY_ORDER_RELAXED);
            m_tail.store(tail + 1, MEMORY_ORDER_RELAXED);
        } else
        {
            constexpr auto memoryOrder = DerivedClass::m_isTotalOrder ? std::memory_order_seq_cst : std::memory_order_relaxed;
            tail = m_tail.fetch_add(1, memoryOrder);
        }
        return static_cast<DerivedClass &>(*this).doPop(tail);
    }

    bool empty() const noexcept
    {
        return !size();
    }

    bool full() const noexcept
    {
        return size() >= static_cast<int32_t>(static_cast<const DerivedClass &>(*this).m_size);
    }

    uint32_t size() const noexcept
    {
        return std::max(static_cast<int32_t>(m_head.load(MEMORY_ORDER_RELAXED) - m_tail.load(MEMORY_ORDER_RELAXED)), 0);
    }

    uint32_t capacity() const noexcept
    {
        return static_cast<int32_t>(static_cast<DerivedClass const &>(*this).m_size);
    }
};

template<class T, uint32_t SIZE, T NIL = nil<T>(), bool WILL_MINIMIZE_CONTENTION = true, bool WILL_MAXIMIZE_THROUGHPUT = true, bool IS_TOTAL_ORDER = false, bool IS_SPSC = false>
class LockFreeQueue : public LockFreeQueueBase<LockFreeQueue<T, SIZE, NIL, WILL_MINIMIZE_CONTENTION, WILL_MAXIMIZE_THROUGHPUT, IS_TOTAL_ORDER, IS_SPSC>> {
    
    using BaseClass = LockFreeQueueBase<LockFreeQueue<T, SIZE, NIL, WILL_MINIMIZE_CONTENTION, WILL_MAXIMIZE_THROUGHPUT, IS_TOTAL_ORDER, IS_SPSC>>;
    friend BaseClass;

    static constexpr uint32_t m_size = WILL_MINIMIZE_CONTENTION ? roundUpToPowerOf2(SIZE) : SIZE;
    static constexpr int32_t SHUFFLE_BITS = GetIndexShuffleBits<WILL_MINIMIZE_CONTENTION, m_size, CACHE_LINE_SIZE / sizeof(std::atomic<T>)>::value;
    static constexpr bool m_isTotalOrder = IS_TOTAL_ORDER;
    static constexpr bool m_isSPSC = IS_SPSC;
    static constexpr bool m_willMaximizeThroughput = WILL_MAXIMIZE_THROUGHPUT;

    alignas(CACHE_LINE_SIZE) std::atomic<T> m_queue[m_size];

    T doPop(uint32_t tail) noexcept
    {
        std::atomic<T> &queueElem = map<SHUFFLE_BITS>(m_queue, tail % m_size);
        return BaseClass::template doPopAtomic<T, NIL>(queueElem);
    }

    void doPush(T element, uint32_t head) noexcept
    {
        std::atomic<T> &queueElem = map<SHUFFLE_BITS>(m_queue, head % m_size);
        BaseClass::template doPushAtomic<T, NIL>(element, queueElem);
    }

public:
    using value_type = T;

    LockFreeQueue() noexcept
    {
        assert(std::atomic<T>{NIL}.is_lock_free());
        for(auto p = m_queue, q = m_queue + m_size; p != q; ++p)
            p->store(NIL, MEMORY_ORDER_RELAXED);
    }

    LockFreeQueue(LockFreeQueue const&) = delete;
    LockFreeQueue& operator=(LockFreeQueue const&) = delete;
    
    friend class LockFreeQueueTest;
	FRIEND_TEST(LockFreeQueueTest, test1);
	FRIEND_TEST(LockFreeQueueTest, test2);
};

} // namespace INTERNAL
} // namespace ITC