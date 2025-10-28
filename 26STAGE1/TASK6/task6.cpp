#include "ThreadPool.h"
#include "fake_camera.hpp"
#include "fake_infer.hpp"
#include "signal.hpp"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

class Vision {
public:
    Vision() {
        pool_ = std::make_unique<ThreadPool>(std::thread::hardware_concurrency());
        camera_ =
            FakeCamera::createUnique(std::bind(&Vision::cameraCallback, this, std::placeholders::_1)
            );
        infer_ =
            FakeInfer::createUnique(std::bind(&Vision::inferCallback, this, std::placeholders::_1));
    }

    ~Vision() {
        stop();
    }

    void stop() {
        std::cout << "Vision destroyed" << std::endl;
        run_flag_ = false;

        if (camera_) {
            camera_->stop();
            camera_.reset();
        }
        if (pool_) {
            pool_->waitUntilEmpty();
            pool_.reset();
        }
    }

    void start() {
        run_flag_ = true;
        camera_->start();
    }

    void cameraCallback(Image& image) {
        camera_count_.fetch_add(1, std::memory_order_relaxed);

        printStatus();

        if (pool_ && run_flag_) {
            //并行推理
            pool_->enqueue([this, frame = std::move(image)]() mutable {
                if (!run_flag_)
                    return;
                if (infer_)
                    infer_->infer(frame);
            });
        }
    }

    void inferCallback(const InferResult& result) {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        track(result);
    }
    void track(const InferResult& result) {
        infer_count_.fetch_add(1, std::memory_order_relaxed);
        static std::chrono::steady_clock::time_point last_timestamp =
            std::chrono::steady_clock::now();

        if (result.timestamp <= last_timestamp) {
            error_count_.fetch_add(1, std::memory_order_relaxed);
            ///std::cout << "Error: Infer result timestamp is not greater than last timestamp" << std::endl;
            return;
        }
        last_timestamp = result.timestamp;
        track_count_.fetch_add(1, std::memory_order_relaxed);
    }

    void printStatus() {
        using namespace std::chrono;
        auto now = steady_clock::now();
        if (duration_cast<seconds>(now - last_print_time_) >= seconds(1)) {
            int cam = camera_count_.exchange(0);
            int inf = infer_count_.exchange(0);
            int err = error_count_.exchange(0);
            int track = track_count_.exchange(0);

            std::cout << "[Status] cameraCallback/s: " << cam << " | inferCallback/s: " << inf

                      << " | track/s: " << track << " | Error/s: " << err << std::endl;
            last_print_time_ = now;
        }
    }

private:
    std::unique_ptr<FakeInfer> infer_;
    std::unique_ptr<FakeCamera> camera_;
    std::unique_ptr<ThreadPool> pool_;

    std::atomic<int> camera_count_ { 0 };
    std::atomic<int> infer_count_ { 0 };
    std::atomic<int> error_count_ { 0 };
    std::atomic<int> track_count_ { 0 };
    std::mutex callback_mutex_;
    std::chrono::steady_clock::time_point last_print_time_ { std::chrono::steady_clock::now() };
    bool run_flag_ = false;
};

int main() {
    try {
        Vision v;
        v.start();

        SignalHandler sig;
        sig.start([&] { v.stop(); });

        while (!sig.shouldExit()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        std::cout << "Exiting program..." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Caught exception in main: " << e.what() << "\n";
        return -1;
    } catch (...) {
        std::cerr << "Unknown exception caught in main!\n";
        return -1;
    }
    return 0;
}
