// Shared HSV(h, 1, 1) -> RGB conversion (full saturation/value, so V-C == 0
// and the "+m" term always drops out). Extracted from RainbowWaveEffect so
// RadialRainbowEffect (Task 4) and later effects (Tasks 6, 8) share one
// implementation instead of each carrying its own copy.
#pragma once

#include <cmath>
#include <cstdint>

#include "protocol/MsiProtocol.h"

// hueDegrees must be in [0,360); callers normalize their own hue formula
// (e.g. via a mod-360 step) before calling this.
inline MsiColor HsvFullToRgb(int hueDegrees)
{
    double c = 1.0;
    double hPrime = hueDegrees / 60.0;
    double x_ = c * (1.0 - std::fabs(std::fmod(hPrime, 2.0) - 1.0));

    double r = 0, g = 0, b = 0;
    if (hueDegrees < 60)       { r = c;  g = x_; b = 0; }
    else if (hueDegrees < 120)  { r = x_; g = c;  b = 0; }
    else if (hueDegrees < 180)  { r = 0;  g = c;  b = x_; }
    else if (hueDegrees < 240)  { r = 0;  g = x_; b = c;  }
    else if (hueDegrees < 300)  { r = x_; g = 0;  b = c;  }
    else                        { r = c;  g = 0;  b = x_; }

    return MsiColor{
        static_cast<uint8_t>(std::lround(r * 255.0)),
        static_cast<uint8_t>(std::lround(g * 255.0)),
        static_cast<uint8_t>(std::lround(b * 255.0))
    };
}
