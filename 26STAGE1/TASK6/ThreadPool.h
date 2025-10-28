#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <stack>
#include <thread>
#include <vector>

// ThreadPool: simple, safe, with a mutex-protected object pool to reduce allocations.
// Features:
//  - Single FIFO task queue (std::queue of TaskItem*).
//  - Mutex + condition_variable protected accesses for both queue and pool.
//  - Thread-safe object pool: acquireNode()/releaseNode() reduce frequent allocations.
//  - enqueue returns bool: true if accepted, false if pool is stopping.
//  - waitUntilEmpty()/pendingTasks() exposed for synchronization/monitoring.
//  - Robust shutdown: stop flag + notify + join; nodes cleaned up safely.

class ThreadPool {
public:
    explicit ThreadPool(size_t num_threads, size_t initial_pool_capacity = 256): stop_(false) {
        if (num_threads == 0)
            num_threads = 1;

        // pre-allocate pool nodes
        for (size_t i = 0; i < initial_pool_capacity; ++i) {
            pool_stack_.push_back(new TaskItem());
        }

        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this] { workerLoop(); });
        }
    }

    ~ThreadPool() {
        // Signal stop and wake all workers
        {
            std::unique_lock<std::mutex> lk(queue_mtx_);
            stop_.store(true);
        }
        cv_.notify_all();

        // join workers
        for (auto& t: workers_) {
            if (t.joinable())
                t.join();
        }

        // After workers joined, the queue must be empty. But to be defensive, clear and free any remaining nodes.
        {
            std::unique_lock<std::mutex> lk(queue_mtx_);
            while (!tasks_.empty()) {
                TaskItem* n = tasks_.front();
                tasks_.pop();
                delete n; // nodes not in pool => free
            }
        }

        // free pool nodes
        {
            std::unique_lock<std::mutex> lk(pool_mtx_);
            for (TaskItem* n: pool_stack_)
                delete n;
            pool_stack_.clear();
        }
    }

    // Enqueue a task. Returns true if accepted; false if pool is stopping.
    template<class F>
    bool enqueue(F&& fn) {
        TaskItem* node = acquireNode();
        if (!node)
            return false; // allocation failure

        node->func = std::function<void()>(std::forward<F>(fn));

        // push into queue under lock; if stopping, release node and return false
        {
            std::unique_lock<std::mutex> lk(queue_mtx_);
            if (stop_.load()) {
                // pool is stopping; recycle node and reject
                lk.unlock();
                releaseNode(node);
                return false;
            }

            tasks_.push(node);
            pending_.fetch_add(1);
        }

        cv_.notify_one();
        return true;
    }

    // Number of tasks waiting or running
    size_t pendingTasks() const {
        return pending_.load();
    }

    // Block until all tasks complete
    void waitUntilEmpty() {
        std::unique_lock<std::mutex> lk(done_mtx_);
        done_cv_.wait(lk, [this] { return pending_.load() == 0 && busy_.load() == 0; });
    }

private:
    struct TaskItem {
        std::function<void()> func;
    };

    // Acquire node from pool (mutex-protected). If pool empty, allocate new node.
    TaskItem* acquireNode() {
        std::lock_guard<std::mutex> lk(pool_mtx_);
        if (!pool_stack_.empty()) {
            TaskItem* n = pool_stack_.back();
            pool_stack_.pop_back();
            return n;
        }
        // fallback allocation
        try {
            return new TaskItem();
        } catch (...) {
            return nullptr;
        }
    }

    // Return node to pool (reset and push back)
    void releaseNode(TaskItem* node) {
        if (!node)
            return;
        node->func = nullptr; // reset
        std::lock_guard<std::mutex> lk(pool_mtx_);
        pool_stack_.push_back(node);
    }

    void workerLoop() {
        while (true) {
            TaskItem* item = nullptr;

            {
                std::unique_lock<std::mutex> lk(queue_mtx_);
                cv_.wait(lk, [this] { return stop_.load() || !tasks_.empty(); });

                if (stop_.load() && tasks_.empty()) {
                    return; // exit
                }

                item = tasks_.front();
                tasks_.pop();
            }

            busy_.fetch_add(1);

            if (item && item->func) {
                try {
                    item->func();
                } catch (...) {
                    // swallow exceptions to keep worker alive
                }
            }

            busy_.fetch_sub(1);
            pending_.fetch_sub(1);

            // notify waiters if empty
            if (pending_.load() == 0 && busy_.load() == 0) {
                std::lock_guard<std::mutex> lk(done_mtx_);
                done_cv_.notify_all();
            }

            // recycle node back to pool
            releaseNode(item);
        }
    }

    // Members
    std::vector<std::thread> workers_;

    // single FIFO task queue (of TaskItem*)
    std::queue<TaskItem*> tasks_;
    mutable std::mutex queue_mtx_;
    std::condition_variable cv_;

    // object pool (mutex protected)
    std::vector<TaskItem*> pool_stack_;
    std::mutex pool_mtx_;

    // counters
    alignas(64) std::atomic<size_t> pending_ { 0 };
    alignas(64) std::atomic<size_t> busy_ { 0 };

    std::atomic<bool> stop_ { false };

    // completion notification
    std::mutex done_mtx_;
    std::condition_variable done_cv_;
};
