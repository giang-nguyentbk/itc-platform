#include "itcLockFreeQueue.h"

#include <cstdint>
#include <string>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

using namespace ITC::INTERNAL;

int64_t toNanoSecondsEpochCount(std::chrono::system_clock::time_point tp) {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        tp.time_since_epoch()).count();
}

int32_t main()
{
    std::string receiverName = "/receiver";
    int32_t receiverShmId = shm_open(receiverName.c_str(), O_RDWR, 0);
    if(receiverShmId < 0)
    {
        std::cout << "ETRUGIA: Failed to shm_open receiverName, errno = " << errno << std::endl;
        return EXIT_FAILURE;
    }
    
    auto receiverShmBasePtr = reinterpret_cast<uint8_t *>(mmap(nullptr, 4096 * 8, PROT_READ | PROT_WRITE, MAP_SHARED, receiverShmId, 0));
    if(!receiverShmBasePtr)
    {
        std::cout << "ETRUGIA: Failed to mmap, errno = " << errno << std::endl;
        return EXIT_FAILURE;
    }
    
    auto shmMessageBasePtr = reinterpret_cast<uint64_t *>(receiverShmBasePtr + (4096 * 4));
    auto m_mpmcReceiverShmQueue = reinterpret_cast<LockFreeQueue<int64_t, 1024, -1, MINIMIZE_CONTENTION, MAXIMIZE_THROUGHPUT, !IS_TOTAL_ORDER, !IS_SPSC> *>(receiverShmBasePtr);
    
    uint32_t NUMBER_OF_MESSAGES {1000};
    int64_t count {0};
    std::chrono::_V2::system_clock::time_point start;
    start = std::chrono::high_resolution_clock::now();
    while(count < NUMBER_OF_MESSAGES)
    {
        std::cout << "ETRUGIA: Sending count = " << count << std::endl;
        if(count == 0)
        {
            shmMessageBasePtr[count] = toNanoSecondsEpochCount(start);
        } else
        {
            shmMessageBasePtr[count] = count;
        }
        m_mpmcReceiverShmQueue->push(count);
        ++count;
    }
    
    munmap(receiverShmBasePtr, 4096 * 8);
    std::cout << "ETRUGIA: Sender done!" << std::endl;
    return EXIT_SUCCESS;
}