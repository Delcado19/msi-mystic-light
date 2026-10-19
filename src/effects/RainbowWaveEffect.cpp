#include "effects/RainbowWaveEffect.h"

#include "effects/HsvToRgb.h"

// Clamp to >=1: periodMs_ is a divisor in colorAt below, so a caller-
// supplied 0 or negative period (e.g. from Task 8's config) would otherwise
// be integer division by zero (SIGFPE), not just a nonsensical hue.
RainbowWaveEffect::RainbowWaveEffect(int64_t periodMs) : periodMs_(periodMs > 0 ? periodMs : 1) {}

MsiColor RainbowWaveEffect::colorAt(int x, int /*y*/, int64_t elapsedMs) const
{
    // hue = ((x * 36) + (elapsedMs * 360 / periodMs_)) mod 360, per the
    // plan's formula (10px wavelength per 360deg, v1). The inner mod/add/mod
    // keeps the result in [0,360) for any sign/magnitude of x or elapsedMs
    // (C++ % can return negative for negative operands).
    int64_t raw = static_cast<int64_t>(x) * 36 + (elapsedMs * 360) / periodMs_;
    int hue = static_cast<int>(((raw % 360) + 360) % 360);

    return HsvFullToRgb(hue);
}
