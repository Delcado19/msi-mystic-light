// Test double for IEffect. Header-only, test-only infrastructure: no
// production code depends on this. Always returns a fixed color regardless
// of position/time, so Canvas tests can assert exactly what was sampled.
#pragma once

#include "effects/IEffect.h"

class FakeEffect : public IEffect {
public:
    explicit FakeEffect(MsiColor c) : color_(c) {}
    MsiColor colorAt(int, int, int64_t) const override { return color_; }
private:
    MsiColor color_;
};
