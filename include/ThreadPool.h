#ifndef DOCUTRACE_THREADPOOL_H
#define DOCUTRACE_THREADPOOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <type_traits>

namespace docutrace {

/**
 * @brief Reusable ThreadPool implementing the Producer-Consumer pattern.
 * Demonstrates Unit V: Concurrency (std::thread, std::mutex, std::condition_variable,
 * std::atomic, std::future, RAII resource management, Deadlock prevention).
 */
class ThreadPool {
private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;

    mutable std::mutex queueMutex_;
    std::condition_variable cvTask_;
    std::condition_variable cvFinished_;

    std::atomic<bool> stop_{false};
    std::atomic<size_t> activeTasks_{0};
    size_t workerCount_{4};

    void workerLoop();

public:
    explicit ThreadPool(size_t threads = std::thread::hardware_concurrency());
    ~ThreadPool();

    // Prevent copying
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    /**
     * @brief Enqueues a callable task and returns a std::future for its result.
     */
    template <class F, class... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::invoke_result<F, Args...>::type> {
        
        using return_type = typename std::invoke_result<F, Args...>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            if (stop_.load()) {
                throw std::runtime_error("Enqueue called on stopped ThreadPool");
            }
            tasks_.emplace([task]() { (*task)(); });
        }
        cvTask_.notify_one();
        return res;
    }

    /**
     * @brief Blocks until all currently queued and running tasks are completed.
     */
    void waitAll();

    size_t getWorkerCount() const noexcept { return workerCount_; }
    size_t getQueueSize() const;
    size_t getActiveTaskCount() const noexcept { return activeTasks_.load(); }
};

} // namespace docutrace

#endif // DOCUTRACE_THREADPOOL_H
