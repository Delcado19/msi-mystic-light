// A horizontal rainbow wave: hue scrolls with x and with time, full
// saturation/value. Wavelength is fixed at 10px/360deg for v1 (not
// configurable -- see colorAt's formula comment in the .cpp).
#pragma once

#include "effects/IEffect.h"

class RainbowWaveEffect : public IEffect {
public:
    explicit RainbowWaveEffect(int64_t periodMs = 3000);
    // Smooth 720-pixel spatial cycle; time advances one full cycle per period.
    // y is ignored for v1 (horizontal wave only).
    MsiColor colorAt(int x, int y, int64_t elapsedMs) const override;

private:
    int64_t periodMs_;
};
