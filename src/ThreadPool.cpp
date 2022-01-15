#include "ThreadPool.hpp"

void ThreadPool::execute()
{
    uint16_t threads_to_spawn = std::min((size_t) max_workers, jobs.size());
    for (uint16_t i = 0; i < threads_to_spawn; ++i)
    {
        thread_pool.emplace_back([&jobs_mutex = jobs_mutex, &jobs = jobs]
        {
            while (!jobs.empty())
            {
                jobs_mutex.lock();
                const auto job = jobs.front();
                jobs.pop();
                jobs_mutex.unlock();
                job(); //Execute the actual task
            }
        });
    }
    for (auto& t : thread_pool) { t.join(); } //Synchronize threads
}
