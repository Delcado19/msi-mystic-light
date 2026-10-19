// A marching-ants style dashed line: dashes of one color separated by black
// gaps, scrolling along x over time. Animated motion, not a user-dragged
// point, so it does not implement IPositionableEffect. y is ignored.
#pragma once

#include "effects/IEffect.h"

class MarqueeEffect : public IEffect {
public:
    MarqueeEffect(MsiColor color, int dashLengthPx = 5, int gapLengthPx = 5, double speedPxPerMs = 0.05);
    // period = dashLengthPx + gapLengthPx (clamped >=1). pos = ((x - elapsedMs*speedPxPerMs)
    // mod period), wrapped into [0, period). on = pos < dashLengthPx.
    // output = on ? color : black.
    MsiColor colorAt(int x, int y, int64_t elapsedMs) const override;

private:
    MsiColor color_;
    int dashLengthPx_, gapLengthPx_;
    double speedPxPerMs_;
};
