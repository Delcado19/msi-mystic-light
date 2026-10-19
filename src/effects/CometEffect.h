// A single bright head that sweeps across a horizontal track and leaves a
// fading tail behind it. Animated motion, not a user-dragged point, so it
// does not implement IPositionableEffect. y is ignored (same convention as
// RainbowWaveEffect).
#pragma once

#include "effects/IEffect.h"

class CometEffect : public IEffect {
public:
    CometEffect(MsiColor color, int trackWidthPx, double speedPxPerMs = 0.05, int tailLengthPx = 20);
    // headX(t) = (elapsedMs * speedPxPerMs) mod trackWidthPx.
    // dist = ((headX - x) mod trackWidthPx), wrapped into [0, trackWidthPx).
    // brightness = max(0, 1 - dist/tailLengthPx). output = color * brightness.
    MsiColor colorAt(int x, int y, int64_t elapsedMs) const override;

private:
    MsiColor color_;
    int trackWidthPx_; // clamped >=1
    double speedPxPerMs_;
    int tailLengthPx_; // clamped >=1
};
