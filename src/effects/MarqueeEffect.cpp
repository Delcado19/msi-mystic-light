#include "effects/MarqueeEffect.h"

#include <algorithm>
#include <cmath>

MarqueeEffect::MarqueeEffect(MsiColor color, int dashLengthPx, int gapLengthPx, double speedPxPerMs)
    : color_(color), dashLengthPx_(dashLengthPx), gapLengthPx_(gapLengthPx), speedPxPerMs_(speedPxPerMs)
{
}

MsiColor MarqueeEffect::colorAt(int x, int /*y*/, int64_t elapsedMs) const
{
    // period is a modulus, so clamp to >=1 here (dash/gap of 0 or negative
    // would otherwise give NaN from fmod). Same reason as CometEffect.
    const double period = static_cast<double>(std::max(1, dashLengthPx_ + gapLengthPx_));

    // Wrap into [0, period) with the same double-precision idiom as CometEffect:
    // the pattern shifts left as time passes, so the raw value goes negative
    // for any x < elapsedMs*speed.
    const double raw = static_cast<double>(x) - static_cast<double>(elapsedMs) * speedPxPerMs_;
    const double pos = std::fmod(std::fmod(raw, period) + period, period);

    const bool on = pos < static_cast<double>(dashLengthPx_);
    return on ? color_ : MsiColor{0, 0, 0};
}
