#pragma once

#include <inttypes.h>
#include <functional>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>


class ThreadPool
{
    private:
        std::vector<std::thread> thread_pool;
        std::queue<std::function<void(void)>> jobs;
        uint16_t max_workers;

        std::mutex jobs_mutex;
    public:
        /**
         * @brief Creates a new thread pool with the specified number of workers.
         * There cannot be more that no_workers threads working on the tasks at the same time.
         */
        ThreadPool(uint16_t max_workers) : max_workers(max_workers) { thread_pool.reserve(max_workers); }

        /**
         * @brief Adds some job to be done. The arguments and the returned type cannot be referenced.
         * Jobs are executed in FIFO (First In First Out) fashion.
         */
        void addWork(std::function<void(void)> job) { jobs.emplace(job); }

        /**
         * @brief Commands all threads to start working! This function joins the threads (is blocking).
         */
        void execute();
};