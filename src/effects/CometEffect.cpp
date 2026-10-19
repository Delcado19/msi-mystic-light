#include "effects/CometEffect.h"

#include <algorithm>
#include <cmath>

// Clamp to >=1: trackWidthPx_ is a modulus and tailLengthPx_ a divisor in
// colorAt below, so a caller-supplied 0 or negative value would be NaN or
// division by zero, same reason as RainbowWaveEffect's constructor.
CometEffect::CometEffect(MsiColor color, int trackWidthPx, double speedPxPerMs, int tailLengthPx)
    : color_(color),
      trackWidthPx_(std::max(1, trackWidthPx)),
      speedPxPerMs_(speedPxPerMs),
      tailLengthPx_(std::max(1, tailLengthPx))
{
}

MsiColor CometEffect::colorAt(int x, int /*y*/, int64_t elapsedMs) const
{
    // fmod keeps the sign of its dividend, so fmod(fmod(v, p) + p, p) is the
    // double-precision version of the ((raw % p) + p) % p idiom used in
    // RainbowWaveEffect, keeping both the head and the distance in [0, track).
    const double track = static_cast<double>(trackWidthPx_);
    const double head = std::fmod(std::fmod(static_cast<double>(elapsedMs) * speedPxPerMs_, track) + track, track);
    const double dist = std::fmod(std::fmod(head - static_cast<double>(x), track) + track, track);

    const double brightness = std::max(0.0, 1.0 - dist / static_cast<double>(tailLengthPx_));

    auto scale = [brightness](uint8_t channel) {
        return static_cast<uint8_t>(std::lround(channel * brightness));
    };
    return MsiColor{scale(color_.r), scale(color_.g), scale(color_.b)};
}
