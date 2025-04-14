#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

#include <gtest/gtest.h>

namespace ITC
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

class ThreadPool
{
public:
    ThreadPool(const ThreadPool &other) = delete;
	ThreadPool &operator=(const ThreadPool &other) = delete;
	ThreadPool(ThreadPool &&other) noexcept = delete;
	ThreadPool &operator=(ThreadPool &&other) noexcept = delete;

    ThreadPool(size_t workerCount)
        : m_isTerminated(false)
    {
        for(size_t i = 0; i < workerCount; ++i)
        {
            m_workers.emplace_back([this]()
            {
                while(1)
                {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(this->m_taskQueueMutex);
                        this->m_condVar.wait(lock, [this]()
                        {
                            return this->m_isTerminated || !this->m_tasks.empty();
                        });
                        if(this->m_isTerminated && this->m_tasks.empty())
                        {
                            return;
                        }
                        task = std::move(this->m_tasks.front());
                        this->m_tasks.pop();
                    }

                    task();
                }
            });
        }
    }
    
    ~ThreadPool()
    {
        terminate();
    }
    
    template<class F, class... Args>
    std::future<typename std::result_of<F(Args...)>::type> enqueue(F&& f, Args&&... args)
    {
        using return_type = typename std::result_of<F(Args...)>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );
            
        std::future<return_type> res = task->get_future();
        
        {
            std::unique_lock<std::mutex> lock(m_taskQueueMutex);
            // prevent enqueueing after terminated
            if(m_isTerminated)
            {
                return std::future<typename std::result_of<F(Args...)>::type>();
            }
            m_tasks.emplace([task](){ (*task)(); });
        }
        m_condVar.notify_one();
        
        return res;
    }
    
    void terminate()
    {
        {
            std::unique_lock<std::mutex> lock(m_taskQueueMutex);
            m_isTerminated = true;
        }
        m_condVar.notify_all();
        for(auto &worker : m_workers)
        {
            worker.join();
        }
    }
    
private:
    std::vector< std::thread > m_workers;
    std::queue< std::function<void()> > m_tasks;
    std::mutex m_taskQueueMutex;
    std::condition_variable m_condVar;
    bool m_isTerminated;
    
    friend class ThreadPoolTest;
	FRIEND_TEST(ThreadPoolTest, test1);
}; // class ThreadPool

} // namespace INTERNAL
} // namespace ITC