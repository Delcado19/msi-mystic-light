#include "effects/RadialRainbowEffect.h"

#include <cmath>

#include "effects/HsvToRgb.h"

namespace {
// Not using M_PI: MSVC only defines it when _USE_MATH_DEFINES is set before
// <cmath> is included, which isn't guaranteed project-wide, so a local
// constant is simpler than relying on that.
constexpr double kPi = 3.14159265358979323846;
}

// Clamp to >=1: periodMs_ is a divisor in colorAt below, same reason as
// RainbowWaveEffect's constructor.
RadialRainbowEffect::RadialRainbowEffect(int centerX, int centerY, int64_t periodMs)
    : centerX_(centerX), centerY_(centerY), periodMs_(periodMs > 0 ? periodMs : 1)
{
}

MsiColor RadialRainbowEffect::colorAt(int x, int y, int64_t elapsedMs) const
{
    // atan2(0,0) is well-defined in C++ (returns 0), so sampling exactly at
    // the center needs no special case -- see
    // testRadialRainbowEffect_zeroDistanceDoesNotCrash.
    double angleRad = std::atan2(static_cast<double>(y - centerY_), static_cast<double>(x - centerX_));
    double angleDeg = angleRad * (180.0 / kPi);

    // Same mod-360 normalization idiom as RainbowWaveEffect: handles
    // atan2's negative range and any sign/magnitude of elapsedMs in one step.
    int64_t raw = static_cast<int64_t>(std::lround(angleDeg)) + (elapsedMs * 360) / periodMs_;
    int hue = static_cast<int>(((raw % 360) + 360) % 360);

    return HsvFullToRgb(hue);
}

void RadialRainbowEffect::setReferencePoint(int x, int y)
{
    centerX_ = x;
    centerY_ = y;
}

std::pair<int,int> RadialRainbowEffect::referencePoint() const
{
    return {centerX_, centerY_};
}
