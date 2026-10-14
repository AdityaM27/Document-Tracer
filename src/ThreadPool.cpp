#include "../include/ThreadPool.h"

namespace docutrace {

ThreadPool::ThreadPool(size_t threads)
    : stop_(false), activeTasks_(0), workerCount_(threads == 0 ? 2 : threads) {
    
    workers_.reserve(workerCount_);
    for (size_t i = 0; i < workerCount_; ++i) {
        workers_.emplace_back(&ThreadPool::workerLoop, this);
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queueMutex_);
        stop_.store(true);
    }
    cvTask_.notify_all();

    for (std::thread& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::workerLoop() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            cvTask_.wait(lock, [this]() {
                return stop_.load() || !tasks_.empty();
            });

            if (stop_.load() && tasks_.empty()) {
                return;
            }

            task = std::move(tasks_.front());
            tasks_.pop();
            activeTasks_++;
        }

        try {
            task();
        } catch (...) {
            // Exceptions within task should not terminate worker thread
        }

        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            activeTasks_--;
            if (activeTasks_.load() == 0 && tasks_.empty()) {
                cvFinished_.notify_all();
            }
        }
    }
}

void ThreadPool::waitAll() {
    std::unique_lock<std::mutex> lock(queueMutex_);
    cvFinished_.wait(lock, [this]() {
        return tasks_.empty() && (activeTasks_.load() == 0);
    });
}

size_t ThreadPool::getQueueSize() const {
    std::unique_lock<std::mutex> lock(queueMutex_);
    return tasks_.size();
}

} // namespace docutrace
