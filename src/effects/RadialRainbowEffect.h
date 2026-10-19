// A rainbow whose hue depends on the angle from a draggable center point
// (vs. RainbowWaveEffect's hue-by-x-position). The center is the effect's
// IPositionableEffect reference point, which Task 14's LightingCanvasWidget
// lets the user drag on the canvas.
#pragma once

#include <utility>

#include "effects/IEffect.h"
#include "effects/IPositionableEffect.h"

class RadialRainbowEffect : public IEffect, public IPositionableEffect {
public:
    explicit RadialRainbowEffect(int centerX = 0, int centerY = 0, int64_t periodMs = 3000);

    // hue = (angleDegrees(x,y around center) + elapsedMs*360/periodMs_) mod
    // 360, full-saturation/value HSV->RGB (shared HsvToRgb.h, same
    // conversion as RainbowWaveEffect). angleDegrees is
    // atan2(y-centerY_, x-centerX_) converted from radians to degrees in
    // [0,360).
    MsiColor colorAt(int x, int y, int64_t elapsedMs) const override;
    void setReferencePoint(int x, int y) override;
    std::pair<int,int> referencePoint() const override;

private:
    int centerX_, centerY_;
    int64_t periodMs_; // clamped >=1 in the constructor, same reason as RainbowWaveEffect
};
