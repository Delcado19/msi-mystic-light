#include "effects/BreathingEffect.h"

#include <cmath>

namespace {
// Not using M_PI: MSVC only defines it when _USE_MATH_DEFINES is set before
// <cmath> is included, same reason as RadialRainbowEffect.cpp.
constexpr double kPi = 3.14159265358979323846;
}

// Clamp to >=1: periodMs_ is a divisor in colorAt below, same reason as
// RainbowWaveEffect's constructor.
BreathingEffect::BreathingEffect(MsiColor color, int64_t periodMs)
    : color_(color), periodMs_(periodMs > 0 ? periodMs : 1)
{
}

MsiColor BreathingEffect::colorAt(int /*x*/, int /*y*/, int64_t elapsedMs) const
{
    double brightness = (std::sin(2.0 * kPi * static_cast<double>(elapsedMs) / static_cast<double>(periodMs_)) + 1.0) / 2.0;

    auto scale = [brightness](uint8_t channel) {
        return static_cast<uint8_t>(std::lround(channel * brightness));
    };
    return MsiColor{scale(color_.r), scale(color_.g), scale(color_.b)};
}
