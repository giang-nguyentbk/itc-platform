#include "itcLockFreeQueue.h"

#include <cstdint>
#include <string>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace ITC::INTERNAL;

int64_t toNanoSecondsEpochCount(std::chrono::system_clock::time_point tp) {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        tp.time_since_epoch()).count();
}

int32_t main()
{
    std::string receiverName = "/receiver";
    shm_unlink(receiverName.c_str());
    int32_t shmId = shm_open(receiverName.c_str(), O_RDWR | O_CREAT | O_EXCL, 0666);
    if(shmId < 0)
    {
        std::cout << "ETRUGIA: Failed to shm_open, errno = " << errno << std::endl;
        return EXIT_FAILURE;
    }
    
    int32_t ret = fchmod(shmId, 0777);
    if(ret < 0)
    {
        std::cout << "ETRUGIA: Failed to fchmod, errno = " << errno << std::endl;
        close(shmId);
        shm_unlink(receiverName.c_str());
        return EXIT_FAILURE;
    }
    
    ret = ftruncate(shmId, 4096 * 8);
    if(ret < 0)
    {
        std::cout << "ETRUGIA: Failed to ftruncate, errno = " << errno << std::endl;
        close(shmId);
        shm_unlink(receiverName.c_str());
        return EXIT_FAILURE;
    }
    
    auto shmBasePtr = reinterpret_cast<uint8_t *>(mmap(nullptr, 4096 * 8, PROT_READ | PROT_WRITE, MAP_SHARED, shmId, 0));
    if(!shmBasePtr)
    {
        std::cout << "ETRUGIA: Failed to mmap, errno = " << errno << std::endl;
        close(shmId);
        shm_unlink(receiverName.c_str());
        return EXIT_FAILURE;
    }
    
    auto shmMessageBasePtr = reinterpret_cast<uint64_t *>(shmBasePtr + (4096 * 4));
    auto m_mpmcShmQueue = new (shmBasePtr) LockFreeQueue<int64_t, 1024, -1, MINIMIZE_CONTENTION, MAXIMIZE_THROUGHPUT, !IS_TOTAL_ORDER, !IS_SPSC>();
    
    uint32_t NUMBER_OF_MESSAGES {1000};
    int64_t count {0};
    std::chrono::_V2::system_clock::time_point end;
    int64_t start {0};
    while(count < NUMBER_OF_MESSAGES)
    {
        auto index = m_mpmcShmQueue->pop();
        std::cout << "ETRUGIA: Waiting on count = " << count << ", index = " << index << ", shmMessageBasePtr[index] = " << shmMessageBasePtr[index] << std::endl;
        if(index == 0)
        {
            start = shmMessageBasePtr[index];
            ++count;
        } else
        {
            if(index >= 0 && shmMessageBasePtr[index] == index)
            {
                ++count;
            }
        }
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "[BENCHMARK] MPMC_Queue_IPC_1000_msgs " << "took " << (toNanoSecondsEpochCount(end) - start) / NUMBER_OF_MESSAGES << " ns\n";
    
    munmap(shmBasePtr, 4096 * 8);
    close(shmId);
    shm_unlink(receiverName.c_str());
    return EXIT_SUCCESS;
}