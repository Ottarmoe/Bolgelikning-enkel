#pragma once
#include <chrono>

uint64_t timeMicroseconds(){
    auto currentTimePoint = std::chrono::steady_clock::now();
    auto microseconds = std::chrono::time_point_cast<std::chrono::microseconds>(currentTimePoint);
    return uint64_t(microseconds.time_since_epoch().count());
}

