#include "ThreadPool.hpp"

void ThreadPool::execute()
{
    for (uint16_t i = 0; i < no_workers; ++i)
    {
        thread_pool.emplace_back([&jobs_mutex = jobs_mutex, &jobs = jobs]
        {
            while (!jobs.empty())
            {
                jobs_mutex.lock();
                auto const& job = jobs.front();
                jobs.pop();
                jobs_mutex.unlock();
                job(); //Execute the actual task
            }
        });
    }
    for (auto& t : thread_pool) { t.join(); } //Synchronize threads
}
