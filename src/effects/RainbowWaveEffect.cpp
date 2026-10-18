#include "effects/RainbowWaveEffect.h"

#include <cmath>

RainbowWaveEffect::RainbowWaveEffect(int64_t periodMs) : periodMs_(periodMs) {}

MsiColor RainbowWaveEffect::colorAt(int x, int /*y*/, int64_t elapsedMs) const
{
    // hue = ((x * 36) + (elapsedMs * 360 / periodMs_)) mod 360, per the
    // plan's formula (10px wavelength per 360deg, v1). The inner mod/add/mod
    // keeps the result in [0,360) for any sign/magnitude of x or elapsedMs
    // (C++ % can return negative for negative operands).
    int64_t raw = static_cast<int64_t>(x) * 36 + (elapsedMs * 360) / periodMs_;
    int hue = static_cast<int>(((raw % 360) + 360) % 360);

    // Standard HSV(h, 1, 1) -> RGB conversion (full saturation/value, so
    // V-C == 0 and the "+m" term always drops out).
    double c = 1.0;
    double hPrime = hue / 60.0;
    double x_ = c * (1.0 - std::fabs(std::fmod(hPrime, 2.0) - 1.0));

    double r = 0, g = 0, b = 0;
    if (hue < 60)       { r = c;  g = x_; b = 0; }
    else if (hue < 120)  { r = x_; g = c;  b = 0; }
    else if (hue < 180)  { r = 0;  g = c;  b = x_; }
    else if (hue < 240)  { r = 0;  g = x_; b = c;  }
    else if (hue < 300)  { r = x_; g = 0;  b = c;  }
    else                 { r = c;  g = 0;  b = x_; }

    return MsiColor{
        static_cast<uint8_t>(std::lround(r * 255.0)),
        static_cast<uint8_t>(std::lround(g * 255.0)),
        static_cast<uint8_t>(std::lround(b * 255.0))
    };
}
