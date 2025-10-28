#pragma once
#include <chrono>
#include <string>
struct Image {
    std::chrono::steady_clock::time_point timestamp;
};

struct InferResult {
    std::chrono::steady_clock::time_point timestamp;
};
