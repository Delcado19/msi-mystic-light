// Concentric rings around a draggable center point. Each ring gets a fixed
// hue step, and the whole pattern rotates over time. Distance (not angle)
// selects the ring, so every point on a circle shares one color. The center
// is the IPositionableEffect reference point.
#pragma once

#include <cstdint>
#include <utility>

#include "effects/IEffect.h"
#include "effects/IPositionableEffect.h"

class SwirlCirclesEffect : public IEffect, public IPositionableEffect {
public:
    explicit SwirlCirclesEffect(int centerX = 0, int centerY = 0, int ringWidthPx = 15, int64_t periodMs = 3000);

    // ringIndex = floor(distance(x,y,center) / ringWidthPx_), and
    // hue = (ringIndex*60 + elapsedMs*360/periodMs_) mod 360, full-saturation/
    // value HSV->RGB (shared HsvToRgb.h). Angle does not enter the color.
    MsiColor colorAt(int x, int y, int64_t elapsedMs) const override;
    void setReferencePoint(int x, int y) override;
    std::pair<int,int> referencePoint() const override;

private:
    int centerX_, centerY_;
    int ringWidthPx_;  // clamped >=1 in the constructor: it is a divisor in colorAt
    int64_t periodMs_; // clamped >=1 in the constructor, same reason as RadialRainbowEffect
};
