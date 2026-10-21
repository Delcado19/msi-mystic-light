#pragma once
#include <chrono>
#include <cstdint>

// One monotonic epoch keeps hardware, desktop and region previews in phase,
// including when a slow initial USB handshake delays the hardware timer.
inline int64_t effectTimeMs() {
    using Clock=std::chrono::steady_clock;
    static const auto epoch=Clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now()-epoch).count();
}
