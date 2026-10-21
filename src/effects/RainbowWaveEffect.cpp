#include "effects/RainbowWaveEffect.h"

#include "effects/HsvToRgb.h"

// Clamp to >=1: periodMs_ is a divisor in colorAt below, so a caller-
// supplied 0 or negative period (e.g. from Task 8's config) would otherwise
// be integer division by zero (SIGFPE), not just a nonsensical hue.
RainbowWaveEffect::RainbowWaveEffect(int64_t periodMs) : periodMs_(periodMs > 0 ? periodMs : 1) {}

MsiColor RainbowWaveEffect::colorAt(int x, int /*y*/, int64_t elapsedMs) const
{
    // A full cycle spans 720 scene pixels, not the old 10-pixel stripes.
    // Fractional hue avoids visible six-step RGB changes at slow speeds.
    const double raw = x * 0.5 + (elapsedMs % periodMs_) * 360.0 / periodMs_;
    const double hue = std::fmod(std::fmod(raw, 360.0) + 360.0, 360.0);

    return HsvFullToRgb(hue);
}
