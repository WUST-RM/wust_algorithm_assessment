#pragma once
#include "common.hpp"
#include <functional>
#include <iostream>
#include <thread>
class FakeCamera {
public:
    using Callback = std::function<void(Image&)>;
    using FakeCameraUnique = std::unique_ptr<FakeCamera>;
    static inline FakeCameraUnique createUnique(Callback cb) {
        return std::make_unique<FakeCamera>(cb);
    }
    FakeCamera(Callback cb) {
        callback_ = cb;
    }
    void stop() {
        run_flag_ = false;
        work_thread_.join();
    }
    void start() {
        run_flag_ = true;
        work_thread_ = std::thread(std::bind(&FakeCamera::workThread, this));
        std::cout << "FakeCamera started" << std::endl;
    }
    void workThread() {
        using clock = std::chrono::steady_clock;
        const std::chrono::microseconds period(4000); // 250 Hz -> 4 ms 周期

        auto next_time = clock::now();
        while (run_flag_) {
            Image image;
            image.timestamp = clock::now();
            callback_(image);

            next_time += period;
            std::this_thread::sleep_until(next_time);
        }
    }

    bool run_flag_ = false;
    Callback callback_;
    std::thread work_thread_;
};