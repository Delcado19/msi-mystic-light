// A spiral: hue combines the angle from a draggable center point with the
// distance from it, plus a time sweep. Unlike RadialRainbowEffect (angle
// only), two points at the same angle but different distances get different
// hues. The center is the IPositionableEffect reference point.
#pragma once

#include <cstdint>
#include <utility>

#include "effects/IEffect.h"
#include "effects/IPositionableEffect.h"

class SpiralEffect : public IEffect, public IPositionableEffect {
public:
    explicit SpiralEffect(int centerX = 0, int centerY = 0, double tightnessDegPerPx = 4.0, int64_t periodMs = 3000);

    // hue = (angleDegrees(x,y around center) + distance(x,y,center)*tightnessDegPerPx_
    //        + elapsedMs*360/periodMs_) mod 360, full-saturation/value HSV->RGB
    // (shared HsvToRgb.h). angleDegrees uses the same convention as
    // RadialRainbowEffect: atan2(y-centerY_, x-centerX_) in degrees, so
    // east=0, south=90, west=180, north=270 on screen.
    // distance 0 (sampling at the center) contributes 0 to the hue, so the
    // color stays defined and deterministic there.
    MsiColor colorAt(int x, int y, int64_t elapsedMs) const override;
    void setReferencePoint(int x, int y) override;
    std::pair<int,int> referencePoint() const override;

private:
    int centerX_, centerY_;
    double tightnessDegPerPx_;
    int64_t periodMs_; // clamped >=1 in the constructor, same reason as RadialRainbowEffect
};
