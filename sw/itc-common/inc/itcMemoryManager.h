#pragma once

#include "itcLockFreeQueue.h"

#include <cstdint>
#include <memory>
#include <functional>
#include <atomic>
#include <initializer_list>
#include <variant>
#include <string>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/ipc.h>
#include <sys/shm.h> 

#include <errno.h>



namespace ITC
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

#define MEMORY_POOL_64_SLOT_SIZE            (uint32_t)(64)
#define MEMORY_POOL_256_SLOT_SIZE           (uint32_t)(256)
#define MEMORY_POOL_512_SLOT_SIZE           (uint32_t)(512)

#define MEMORY_POOL_PAGE_SIZE               (uint32_t)(4096)
#define ROUND_UP_TO_64_BYTES(x)             (((x) + 63) & ~63)
#define ROUND_UP_TO_PAGE_SIZES(x)           (((x) + MEMORY_POOL_PAGE_SIZE - 1) & ~(MEMORY_POOL_PAGE_SIZE - 1))

// 
/***
 * + 6 x 64 bytes   :   MEMORY_POOL_64_METADATA             = (384 bytes)       }
 * + 3 x 64 bytes   :   MEMORY_POOL_256_METADATA            = (192 bytes)       }
 * + 3 x 64 bytes   :   MEMORY_POOL_512_METADATA            = (192 bytes)       } -> (4096 bytes)
 * + 1 x 64 bytes   :   MEMORY_POOL_UNLIMITED_METADATA      = (64 bytes)        }
 * + 51 x 64 bytes  :   MEMORY_POOL_64_DATA                 = (3264 bytes)      }
 * + 16 x 256 bytes :   MEMORY_POOL_256_DATA                = (4096 bytes)
 * + 16 x 512 bytes :   MEMORY_POOL_512_DATA                = (8192 bytes)
 * -> 4 pages = 16KB
 */

#define MEMORY_POOL_64_SLOTS                (uint32_t)(51)
#define MEMORY_POOL_256_SLOTS               (uint32_t)(16)
#define MEMORY_POOL_512_SLOTS               (uint32_t)(16)
#define MEMORY_POOL_UNLIMITED_SLOTS         (uint32_t)(1)

#define MEMORY_POOL_64_METADATA_SIZE        (uint32_t)ROUND_UP_TO_64_BYTES((sizeof(int32_t) * MEMORY_POOL_64_SLOTS) + 128) /* 128 is for LockFreeQueueBase's head and tail. */
#define MEMORY_POOL_256_METADATA_SIZE       (uint32_t)((sizeof(int32_t) * MEMORY_POOL_256_SLOTS) + 128) /* 128 is for LockFreeQueueBase's head and tail. */
#define MEMORY_POOL_512_METADATA_SIZE       (uint32_t)((sizeof(int32_t) * MEMORY_POOL_512_SLOTS) + 128) /* 128 is for LockFreeQueueBase's head and tail. */
#define MEMORY_POOL_UNLIMITED_METADATA_SIZE (uint32_t)ROUND_UP_TO_64_BYTES(sizeof(std::atomic<int32_t>))

#define MEMORY_POOL_METADATA_SIZE           (uint32_t)(MEMORY_POOL_64_METADATA_SIZE + MEMORY_POOL_256_METADATA_SIZE + MEMORY_POOL_512_METADATA_SIZE + MEMORY_POOL_UNLIMITED_METADATA_SIZE)


#define MEMORY_POOL_64_TOTAL_SIZE               (uint32_t)(MEMORY_POOL_64_SLOTS * MEMORY_POOL_64_SLOT_SIZE)
#define MEMORY_POOL_256_TOTAL_SIZE              (uint32_t)(MEMORY_POOL_256_SLOTS * MEMORY_POOL_256_SLOT_SIZE)
#define MEMORY_POOL_512_TOTAL_SIZE              (uint32_t)(MEMORY_POOL_512_SLOTS * MEMORY_POOL_512_SLOT_SIZE)
#define MEMORY_POOL_TOTAL_SIZE                  (uint32_t)(MEMORY_POOL_METADATA_SIZE + MEMORY_POOL_64_TOTAL_SIZE + MEMORY_POOL_256_TOTAL_SIZE + MEMORY_POOL_512_TOTAL_SIZE)

#define MEMORY_POOL_64_START_OFFSET             (uint32_t)(MEMORY_POOL_METADATA_SIZE)
#define MEMORY_POOL_256_START_OFFSET            (uint32_t)(MEMORY_POOL_64_START_OFFSET + MEMORY_POOL_64_TOTAL_SIZE)
#define MEMORY_POOL_512_START_OFFSET            (uint32_t)(MEMORY_POOL_256_START_OFFSET + MEMORY_POOL_256_TOTAL_SIZE)
// #define MEMORY_POOL_UNLIMITED_START_OFFSET      (uint32_t)(MEMORY_POOL_512_START_OFFSET + MEMORY_POOL_512_TOTAL_SIZE)

#define MEMORY_POOL_MODE_NO_POOLING             (uint32_t)(0)
#define MEMORY_POOL_MODE_SYSV_SHARED            (uint32_t)(1)
#define MEMORY_POOL_MODE_POSIX_SHARED           (uint32_t)(2)
#define MEMORY_POOL_MODE_THREAD_SHARED          (uint32_t)(3)

/***
 * 1. new/delete                    align 16                        tất cả threads tự allocate msg, rx queue đặt ở đâu cũng được, rx queue chứa absolute address
 * 2. mmap(ANONYMOUS)               align 4096                      tất cả threads cùng vào shopping 1 pool, rx queue đặt ở đâu cũng được, rx queue chứa absolute address, pool unlimited bắt là một mmap(ANONYMOUS) khác -> push vào là absolute address
 * 3. mmap(SHARED)                  align 4096                      tất cả processes cùng vào shopping receiver's pool, mỗi process 1 rx queue + 1 mempool, rx queue bắt buộc đặt ở trên mempool, rx queue chứa offset, pool unlimited bắt đầu từ offset lớn nhất và được ftruncate lên -> push vào là offset
 * 3. shmat                         align 4096                      tất cả processes cùng vào shopping receiver's pool, mỗi process 1 rx queue + 1 mempool, rx queue bắt buộc đặt ở trên mempool, rx queue chứa offset, pool unlimited là một file name khác đã được cả receiver và sender biết trước -> push vào 1 predefined negative value like -10, -20, -30,...
 */

using UInt8RawPtr = uint8_t *;

class SharedMemoryIf
{
public:
    SharedMemoryIf(const SharedMemoryIf &other) = delete;
    SharedMemoryIf &operator=(const SharedMemoryIf &other) = delete;
    SharedMemoryIf(SharedMemoryIf &&other) noexcept = delete;
    SharedMemoryIf &operator=(SharedMemoryIf &&other) noexcept = delete;
    
    virtual UInt8RawPtr get() = 0;
    virtual bool append(uint32_t size) = 0;
    
protected:
    SharedMemoryIf() = default;
    SharedMemoryIf(uint32_t size, UInt8RawPtr basePtr = nullptr)
        : m_size(size),
          m_basePtr(basePtr)
    {}
    virtual ~SharedMemoryIf() = default;
    
protected:
    uint32_t m_size {0};
    UInt8RawPtr m_basePtr {nullptr};
};

class SYSVSharedMemory : public SharedMemoryIf
{
public:
    SYSVSharedMemory(const std::string &shmName, int32_t projId, uint32_t size)
        : SharedMemoryIf(ROUND_UP_TO_PAGE_SIZES(size))
    {
        key_t shmKey = ftok(shmName.c_str(), projId);
        if(shmKey < 0)
        {
            return;
        }

        m_shmId = shmget(shmKey, m_size, 0666 | IPC_CREAT | IPC_EXCL);
        if(m_shmId < 0)
        {
            return;
        }

        m_basePtr = reinterpret_cast<UInt8RawPtr>(shmat(m_shmId, NULL, 0));
    }
    ~SYSVSharedMemory()
    {
        if(m_basePtr)
        {
            shmdt(reinterpret_cast<void *>(m_basePtr));
        }
        if(m_shmId > 0)
        {
            shmctl(m_shmId, IPC_RMID, NULL);
        }
    }
    
    UInt8RawPtr get() override
    {
        return m_basePtr;
    }
    
    bool append(uint32_t size) override
    {
        
    }

private:
    int32_t m_shmId {-1};
};

class POSIXSharedMemory : public SharedMemoryIf
{
public:
    POSIXSharedMemory(const std::string &shmName, uint32_t size)
        : m_shmName(shmName),
          SharedMemoryIf(ROUND_UP_TO_PAGE_SIZES(size))
    {
        int32_t shmId = shm_open(m_shmName.c_str(), O_RDWR | O_CREAT | O_EXCL, 0666);
        if(shmId < 0 && errno != EEXIST)
        {
            std::cout << "ETRUGIA: Failed to shm_open, errno = " << errno << std::endl;
            return;
        }
        
        int32_t ret = fchmod(shmId, 0777);
        if(ret < 0)
        {
            std::cout << "ETRUGIA: Failed to fchmod, errno = " << errno << std::endl;
            close(shmId);
            shm_unlink(m_shmName.c_str());
            return;
        }
        
        ret = ftruncate(shmId, m_size);
        if(ret < 0)
        {
            std::cout << "ETRUGIA: Failed to ftruncate, errno = " << errno << std::endl;
            close(shmId);
            shm_unlink(m_shmName.c_str());
            return;
        }
        
        m_basePtr = reinterpret_cast<UInt8RawPtr>(mmap(nullptr, m_size, PROT_READ | PROT_WRITE, MAP_SHARED, shmId, 0));
    }
    ~POSIXSharedMemory()
    {
        if(m_basePtr)
        {
            munmap(m_basePtr, m_size);
        }
        if(m_shmId > 0)
        {
            close(m_shmId);
        }
        shm_unlink(m_shmName.c_str());
    }
    
    UInt8RawPtr get()
    {
        return m_basePtr;
    }

private:
    std::string m_shmName;
    int32_t m_shmId {-1};
};

class ThreadSharedMemory : public SharedMemoryIf
{
public:
    ThreadSharedMemory(uint32_t size)
        : SharedMemoryIf(ROUND_UP_TO_PAGE_SIZES(size))
    {
        m_basePtr = reinterpret_cast<UInt8RawPtr>(mmap(nullptr, m_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
    }
    ~ThreadSharedMemory()
    {
        if(m_basePtr)
        {
            munmap(m_basePtr, m_size);
        }
    }
    
    UInt8RawPtr get()
    {
        return m_basePtr;
    }

private:
    UInt8RawPtr m_basePtr {nullptr};
    uint32_t m_size {0};
};

struct MemoryChunkInfo
{
    uint64_t offset {0}; /* This is an offset "popped out from"/"pushed back into" "memory pool's queues"/"rx message queues". */
    UInt8RawPtr baseAddr {nullptr}; /* This is used along with offset to calculate absolute address of slots. */
};

#define MEMORY_ALLOCATOR_MODE_1             (uint32_t)(1)
#define MEMORY_ALLOCATOR_MODE_2             (uint32_t)(2)
#define MEMORY_ALLOCATOR_MODE_3             (uint32_t)(3)
#define MEMORY_ALLOCATOR_MODE_4             (uint32_t)(4)

/* 1. ThreadSharedMemory1: trivial new[]/delete[] */
struct Mode1Attributes
{
    UInt8RawPtr baseAddr {nullptr};
    
    void reset()
    {
        baseAddr = nullptr;
    }
};

/* 2. ThreadSharedMemory2: mmap(MAP_PRIVATE | MAP_ANONYMOUS). */
struct Mode2Attributes
{
    UInt8RawPtr baseAddr {nullptr};
    uint32_t size {0};
    
    void reset()
    {
        baseAddr = nullptr;
        size = 0;
    }
};

/* 3. POSIXSharedMemory: mmap(MAP_SHARED). */
struct Mode3Attributes
{
    UInt8RawPtr baseAddr {nullptr};
    uint32_t size {0};
    UInt8RawPtr appendedBaseAddress {nullptr};
    uint32_t appendedSize {0};
    int32_t shmId {-1};
    uint32_t isOwner {0}; /* 1: owner who created the POSIX Shared Memory object. 0: we are opening only. */
    std::string shmName;
    
    void reset()
    {
        baseAddr = nullptr;
        appendedBaseAddress = nullptr;
        size = 0;
        appendedSize = 0;
        shmId = -1;
        isOwner = 0;
        shmName = "";
    }
};

/* 3. SYSVSharedMemory: shmat(). */
struct Mode4Attributes
{
    UInt8RawPtr baseAddr {nullptr};
    uint32_t size {0};
    int32_t shmId {-1};
    int32_t projId {-1};
    uint32_t isOwner {0}; /* 1: owner who created the POSIX Shared Memory object. 0: we are opening only. */
    std::string shmName;
    
    void reset()
    {
        baseAddr = nullptr;
        size = 0;
        shmId = -1;
        projId = -1;
        isOwner = 0;
        shmName = "";
    }
};

struct MemoryAllocatorParams
{
    uint32_t mode {MEMORY_ALLOCATOR_MODE_1};
    union
    {
        Mode1Attributes mode1;
        Mode2Attributes mode2;
        Mode3Attributes mode3;
        Mode4Attributes mode4;
    } attrs;
    
};


class MemoryAllocator
{
public:
    static UInt8RawPtr allocate(uint32_t size, MemoryAllocatorParams &params)
    {
        if(params.mode == MEMORY_ALLOCATOR_MODE_1)
        {
            params.attrs.mode1.baseAddr = new uint8_t[size];
            return params.attrs.mode1.baseAddr;
        } else if(params.mode == MEMORY_ALLOCATOR_MODE_2)
        {
            params.attrs.mode2.baseAddr = reinterpret_cast<UInt8RawPtr>(mmap(nullptr, ROUND_UP_TO_PAGE_SIZES(size), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
            params.attrs.mode2.size = ROUND_UP_TO_PAGE_SIZES(size);
            return params.attrs.mode2.baseAddr;
        } else if(params.mode == MEMORY_ALLOCATOR_MODE_3)
        {
            uint32_t isOwner {1};
            int32_t shmId = shm_open(params.attrs.mode3.shmName.c_str(), O_RDWR | O_CREAT | O_EXCL, 0666);
            if(shmId < 0)
            {
                std::cout << "ETRUGIA: Failed to shm_open, errno = " << errno << std::endl;
                if(errno != EEXIST)
                {
                    return nullptr;
                }
                
                shmId = shm_open(params.attrs.mode3.shmName.c_str(), O_RDWR, 0666);
                if(shmId < 0)
                {
                    return nullptr;
                }
                
                isOwner = 0; /* We are not the owner who created this shared object. */
            }
            
            int32_t ret = fchmod(shmId, 0777);
            if(ret < 0)
            {
                std::cout << "ETRUGIA: Failed to fchmod, errno = " << errno << std::endl;
                if(isOwner)
                {
                    close(shmId);
                    shm_unlink(params.attrs.mode3.shmName.c_str());
                }
                return nullptr;
            }
            
            ret = ftruncate(shmId, ROUND_UP_TO_PAGE_SIZES(size));
            if(ret < 0)
            {
                std::cout << "ETRUGIA: Failed to ftruncate, errno = " << errno << std::endl;
                if(isOwner)
                {
                    close(shmId);
                    shm_unlink(params.attrs.mode3.shmName.c_str());
                }
                return nullptr;
            }
            
            params.attrs.mode3.baseAddr = reinterpret_cast<UInt8RawPtr>(mmap(nullptr, ROUND_UP_TO_PAGE_SIZES(size), PROT_READ | PROT_WRITE, MAP_SHARED, shmId, 0));
            params.attrs.mode3.size = ROUND_UP_TO_PAGE_SIZES(size);
            params.attrs.mode3.shmId = shmId;
            params.attrs.mode3.isOwner = isOwner;
            return params.attrs.mode3.baseAddr;
        } else if(params.mode == MEMORY_ALLOCATOR_MODE_4)
        {
            uint32_t isOwner {1};
            key_t shmKey = ftok(params.attrs.mode4.shmName.c_str(), params.attrs.mode4.projId);
            if(shmKey < 0)
            {
                return nullptr;
            }

            int32_t shmId = shmget(shmKey, ROUND_UP_TO_PAGE_SIZES(size), 0666 | IPC_CREAT | IPC_EXCL);
            if(shmId < 0)
            {
                if(errno != EEXIST)
                {
                    return nullptr;
                }
                
                shmId = shmget(shmKey, ROUND_UP_TO_PAGE_SIZES(size), 0666);
                if(shmId < 0)
                {
                    return nullptr;
                }
                
                isOwner = 0;
            }

            params.attrs.mode4.baseAddr = reinterpret_cast<UInt8RawPtr>(shmat(shmId, nullptr, 0));
            params.attrs.mode4.size = ROUND_UP_TO_PAGE_SIZES(size);
            params.attrs.mode4.shmId = shmId;
            params.attrs.mode4.isOwner = isOwner;
            return params.attrs.mode4.baseAddr;
        }
        
        return nullptr;
    }

    static UInt8RawPtr append(uint32_t size, MemoryAllocatorParams &params)
    {
        if(params.mode == MEMORY_ALLOCATOR_MODE_3)
        {
            if(params.attrs.mode3.appendedBaseAddress)
            {
                return nullptr;
            }
            
            int32_t ret = ftruncate(params.attrs.mode3.shmId, params.attrs.mode3.size + ROUND_UP_TO_PAGE_SIZES(size));
            if(ret < 0)
            {
                std::cout << "ETRUGIA: Failed to ftruncate, errno = " << errno << std::endl;
                if(params.attrs.mode3.isOwner)
                {
                    close(params.attrs.mode3.shmId);
                    shm_unlink(params.attrs.mode3.shmName.c_str());
                }
                return nullptr;
            }
            
            params.attrs.mode3.appendedBaseAddress = reinterpret_cast<UInt8RawPtr>(mmap(nullptr, ROUND_UP_TO_PAGE_SIZES(size), PROT_READ | PROT_WRITE, MAP_SHARED, params.attrs.mode3.shmId, params.attrs.mode3.size));
            params.attrs.mode3.appendedSize = ROUND_UP_TO_PAGE_SIZES(size);
        } else if(params.mode == MEMORY_ALLOCATOR_MODE_1 || params.mode == MEMORY_ALLOCATOR_MODE_2 || params.mode == MEMORY_ALLOCATOR_MODE_4)
        {
            return allocate(size, params);
        }
        
        return nullptr;
    }

    static void deallocate(MemoryAllocatorParams &params)
    {
        if(params.mode == MEMORY_ALLOCATOR_MODE_1)
        {
            if(params.attrs.mode1.baseAddr)
            {
                delete[] params.attrs.mode1.baseAddr;
                params.attrs.mode1.reset();
            }
        } else if(params.mode == MEMORY_ALLOCATOR_MODE_2)
        {
            if(params.attrs.mode2.baseAddr)
            {
                munmap(params.attrs.mode2.baseAddr, params.attrs.mode2.size);
                params.attrs.mode2.reset();
            }
        } else if(params.mode == MEMORY_ALLOCATOR_MODE_3 && params.attrs.mode3.isOwner)
        {
            if(params.attrs.mode3.baseAddr)
            {
                munmap(params.attrs.mode3.baseAddr, params.attrs.mode3.size);
            }
            if(params.attrs.mode3.appendedBaseAddress)
            {
                munmap(params.attrs.mode3.appendedBaseAddress, params.attrs.mode3.appendedSize);
            }
            if(params.attrs.mode3.shmId > 0)
            {
                close(params.attrs.mode3.shmId);
            }
            shm_unlink(params.attrs.mode3.shmName.c_str());
            params.attrs.mode3.reset();
        } else if(params.mode == MEMORY_ALLOCATOR_MODE_4 && params.attrs.mode4.isOwner)
        {
            if(params.attrs.mode4.baseAddr)
            {
                shmdt(reinterpret_cast<void *>(params.attrs.mode4.baseAddr));
            }
            if(params.attrs.mode4.shmId > 0)
            {
                shmctl(params.attrs.mode4.shmId, IPC_RMID, NULL);
            }
            params.attrs.mode4.reset();
        }
    }
};

class MemoryPool
{
    using MemoryPool64Queue = LockFreeQueue<int32_t /* 64-byte index from starting of pool 64 */, MEMORY_POOL_64_SLOTS, -1, MINIMIZE_CONTENTION, MAXIMIZE_THROUGHPUT, !IS_TOTAL_ORDER, !IS_SPSC>;
    using MemoryPool256Queue = LockFreeQueue<int32_t /* 64-byte index from starting of pool 256 */, MEMORY_POOL_256_SLOTS, -1, MINIMIZE_CONTENTION, MAXIMIZE_THROUGHPUT, !IS_TOTAL_ORDER, !IS_SPSC>;
    using MemoryPool512Queue = LockFreeQueue<int32_t /* 64-byte index from starting of pool 512 */, MEMORY_POOL_512_SLOTS, -1, MINIMIZE_CONTENTION, MAXIMIZE_THROUGHPUT, !IS_TOTAL_ORDER, !IS_SPSC>;
    using MemoryPoolUnlimitedLock = std::atomic<int32_t /* 64-byte index from starting of pool unlimited */>;
    using MemoryPool64QueueRawPtr = MemoryPool64Queue *;
    using MemoryPool256QueueRawPtr = MemoryPool256Queue *;
    using MemoryPool512QueueRawPtr = MemoryPool512Queue *;
    using MemoryPoolUnlimitedLockRawPtr = MemoryPoolUnlimitedLock *;
    
    MemoryPool(UInt8RawPtr baseAddr, uint32_t size)
        : m_baseAddr(baseAddr)
    {
        if(size < MEMORY_POOL_TOTAL_SIZE)
        {
            return;
        }
        
        /* Put pool metadata on allocated memory. */
        m_pool64 = new (m_baseAddr) MemoryPool64Queue();
        m_pool256 = new (m_pool64 + MEMORY_POOL_64_METADATA_SIZE) MemoryPool256Queue();
        m_pool512 = new (m_pool256 + MEMORY_POOL_256_METADATA_SIZE) MemoryPool512Queue();
        m_poolUnlimited = new (m_pool512 + MEMORY_POOL_UNLIMITED_METADATA_SIZE) MemoryPoolUnlimitedLock();
        
        /* Initialise pool slots. */
        for(int32_t i = 0; i < static_cast<uint32_t>(MEMORY_POOL_64_SLOTS); ++i)
        {
            m_pool64->push(MEMORY_POOL_64_START_OFFSET + i * MEMORY_POOL_64_SLOT_SIZE);
        }
        for(int32_t i = 0; i < static_cast<uint32_t>(MEMORY_POOL_256_SLOTS); ++i)
        {
            m_pool256->push(MEMORY_POOL_256_START_OFFSET + i * MEMORY_POOL_256_SLOT_SIZE);
        }
        for(int32_t i = 0; i < static_cast<uint32_t>(MEMORY_POOL_512_SLOTS); ++i)
        {
            m_pool512->push(MEMORY_POOL_512_START_OFFSET + i * MEMORY_POOL_512_SLOT_SIZE);
        }
    }
    
    ~MemoryPool()
    {}
    
    UInt8RawPtr allocate(uint32_t size)
    {
        if(size <= MEMORY_POOL_64_SLOT_SIZE)
        {
            auto index = m_pool64->pop();
            return reinterpret_cast<UInt8RawPtr>(m_baseAddr + MEMORY_POOL_64_START_OFFSET + index * MEMORY_POOL_64_SLOT_SIZE);
        } else if(size <= MEMORY_POOL_256_SLOT_SIZE)
        {
            auto index = m_pool256->pop();
            return reinterpret_cast<UInt8RawPtr>(m_baseAddr + MEMORY_POOL_256_START_OFFSET + index * MEMORY_POOL_256_SLOT_SIZE);
        } else if(size <= MEMORY_POOL_512_SLOT_SIZE)
        {
            auto index = m_pool512->pop();
            return reinterpret_cast<UInt8RawPtr>(m_baseAddr + MEMORY_POOL_512_START_OFFSET + index * MEMORY_POOL_512_SLOT_SIZE);
        }
        return nullptr;
    }
    
    void deallocate(uint32_t size)
    {
        if(m_mode == MEMORY_POOL_MODE_NO_POOLING)
        {
            return new uint8_t[size];
        } else if (m_mode == MEMORY_POOL_MODE_SYSV_SHARED
            || m_mode == MEMORY_POOL_MODE_POSIX_SHARED
            || m_mode == MEMORY_POOL_MODE_THREAD_SHARED)
        {
            if(size <= MEMORY_POOL_64_SLOT_SIZE)
            {
                auto index = m_pool64->pop();
                return reinterpret_cast<UInt8RawPtr>(m_baseAddr + MEMORY_POOL_64_START_OFFSET + index * MEMORY_POOL_64_SLOT_SIZE);
            } else if(size <= MEMORY_POOL_256_SLOT_SIZE)
            {
                auto index = m_pool256->pop();
                return reinterpret_cast<UInt8RawPtr>(m_baseAddr + MEMORY_POOL_256_START_OFFSET + index * MEMORY_POOL_256_SLOT_SIZE);
            } else if(size <= MEMORY_POOL_512_SLOT_SIZE)
            {
                auto index = m_pool512->pop();
                return reinterpret_cast<UInt8RawPtr>(m_baseAddr + MEMORY_POOL_512_START_OFFSET + index * MEMORY_POOL_512_SLOT_SIZE);
            }
        }
        return nullptr;
    }

private:

    struct NoDelete
    {
        void operator()(void *) const noexcept
        {}
    };
    

private:
    uint32_t m_mode;
    UInt8RawPtr m_baseAddr {nullptr};
    MemoryPool64QueueRawPtr m_pool64 {nullptr}; /* 4KB */
    MemoryPool256QueueRawPtr m_pool256 {nullptr}; /* 8KB */
    MemoryPool512QueueRawPtr m_pool512 {nullptr}; /* 8KB */
    
    MemoryPoolUnlimitedLockRawPtr m_poolUnlimited {nullptr};
    uint32_t m_poolUnlimitedStartOffset {0};
}; // class MemoryPool

class MemoryManager
{

private:
    MemoryPool m_memPool;
    
};


} // namespace INTERNAL
} // namespace ITC