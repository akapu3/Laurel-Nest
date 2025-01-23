#include "threadpool.hpp"

ThreadPool::ThreadPool(size_t numThreads) : stop(false), activeTasks(0) {
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back([this]() {
            while (true) {
                std::function<void()> task;

                {
                    std::unique_lock<std::mutex> lock(queueMutex);
                    condition.wait(lock, [this]() { return stop || !tasks.empty(); });

                    if (stop && tasks.empty()) {
                        return;
                    }

                    task = std::move(tasks.front());
                    tasks.pop();
                    ++activeTasks;
                }

                task();

                {
                    std::lock_guard<std::mutex> lock(queueMutex);
                    --activeTasks;
                    if (tasks.empty() && activeTasks == 0) {
                        done.notify_all();
                    }
                }
            }
            });
    }
}

ThreadPool::~ThreadPool() {
    stop = true;
    condition.notify_all();

    for (std::thread& worker : workers) {
        worker.join();
    }
}

void ThreadPool::enqueue(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        tasks.emplace(std::move(task));
    }
    condition.notify_one();
}

void ThreadPool::waitForCompletion() {
    std::unique_lock<std::mutex> lock(queueMutex);
    done.wait(lock, [this]() { return tasks.empty() && activeTasks == 0; });
}

size_t ThreadPool::getNumThreads() {
  return workers.size();
}
