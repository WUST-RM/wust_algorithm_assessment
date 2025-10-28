#pragma once
#include "common.hpp"
#include <functional>
#include <memory>
#include <random>
#include <thread>

class FakeInfer {
public:
    using InferCallback = std::function<void(InferResult)>;
    using FakeInferUnique = std::unique_ptr<FakeInfer>;
    static inline FakeInferUnique createUnique(InferCallback cb) {
        return std::make_unique<FakeInfer>(cb);
    }
    FakeInfer(InferCallback cb) {
        callback_ = cb;
    }
    void infer(const Image& image) {
        // 随机生成 10~20 ms 的延迟
        static thread_local std::mt19937 rng(std::random_device {}());
        std::uniform_int_distribution<int> dist(10, 20);
        int delay_ms = dist(rng);

        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));

        callback_(InferResult { image.timestamp });
    }
    InferCallback callback_;
};