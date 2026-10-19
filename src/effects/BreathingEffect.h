// A uniform pulse: one color, brightness rises and falls sinusoidally over
// periodMs. Ignores x and y (same color everywhere). Used standalone, with
// no spatial reference point.
#pragma once

#include "effects/IEffect.h"

class BreathingEffect : public IEffect {
public:
    explicit BreathingEffect(MsiColor color, int64_t periodMs = 3000);
    // brightness(t) = (sin(2*pi*elapsedMs/periodMs_) + 1) / 2, applied
    // uniformly to color_ (each channel is scaled and rounded).
    MsiColor colorAt(int x, int y, int64_t elapsedMs) const override;

private:
    MsiColor color_;
    int64_t periodMs_;
};
