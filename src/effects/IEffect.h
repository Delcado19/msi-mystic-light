// Effect interface: pure render-time color computation. An IEffect must be
// a pure function of (x, y, elapsedMs) -- no hidden mutable state, no I/O,
// no dependency on anything beyond MsiColor. Canvas (Task 8) calls colorAt
// per-LED per-frame to decide what to push to the device.
#pragma once

#include <cstdint>

#include "protocol/MsiProtocol.h"

class IEffect {
public:
    virtual ~IEffect() = default;
    // Color this effect wants at local coordinate (x,y) at elapsedMs since
    // the effect was (re)started.
    virtual MsiColor colorAt(int x, int y, int64_t elapsedMs) const = 0;
};
