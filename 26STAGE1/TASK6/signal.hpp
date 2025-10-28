#pragma once
// #include "logger.hpp"
#include <atomic>
#include <condition_variable>
#include <csignal>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <mutex>
#include <ostream>
#include <thread>
class SignalHandler {
public:
    SignalHandler() = default;
    ~SignalHandler() {
        if (wait_thread_.joinable()) {
            wait_thread_.join();
        }
    }

    /// 注册 SIGINT 信号，并启动后台监控线程
    void start(std::function<void()> on_exit = nullptr) {
        on_exit_ = std::move(on_exit);

        std::signal(SIGINT, SignalHandler::handleSignal);

        instance_ = this; // 保存单例指针，用于静态回调

        wait_thread_ = std::thread([this] {
            std::unique_lock<std::mutex> lk(mtx_);
            cv_.wait(lk, [this] { return exit_flag_.load(std::memory_order_acquire); });
            if (on_exit_) {
                on_exit_(); // 调用退出回调
            }
        });
    }

    /// 设置退出标志，通知等待线程
    void requestExit() {
        exit_flag_.store(true, std::memory_order_release);
        cv_.notify_one();
    }

    /// 判断是否退出
    bool shouldExit() const {
        return exit_flag_.load(std::memory_order_acquire);
    }

private:
    static void handleSignal(int signum) {
        if (!instance_)
            return;
        if (!instance_->sigint_received_.exchange(true)) {
            // 第一次收到 Ctrl+C
            std::cout << "Interrupt signal (" << signum << ") received. Exiting gracefully..."
                      << std::endl;
            instance_->requestExit();
        } else {
            // 第二次收到 Ctrl+C
            std::cout << "Interrupt signal (" << signum << ") received again. Forcing exit."
                      << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    static inline SignalHandler* instance_ = nullptr; // 静态指针用于 signal 回调

    std::atomic<bool> exit_flag_ { false };
    std::atomic<bool> sigint_received_ { false };
    std::mutex mtx_;
    std::condition_variable cv_;
    std::thread wait_thread_;
    std::function<void()> on_exit_;
};
