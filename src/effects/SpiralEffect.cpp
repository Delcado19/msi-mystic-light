#include "effects/SpiralEffect.h"

#include <cmath>

#include "effects/HsvToRgb.h"

namespace {
// Local constant, same reason as RadialRainbowEffect.cpp: M_PI is not
// reliably defined on MSVC without _USE_MATH_DEFINES.
constexpr double kPi = 3.14159265358979323846;
}

// Clamp to >=1: periodMs_ is a divisor in colorAt, same reason as
// RadialRainbowEffect's constructor.
SpiralEffect::SpiralEffect(int centerX, int centerY, double tightnessDegPerPx, int64_t periodMs)
    : centerX_(centerX), centerY_(centerY), tightnessDegPerPx_(tightnessDegPerPx), periodMs_(periodMs > 0 ? periodMs : 1)
{
}

MsiColor SpiralEffect::colorAt(int x, int y, int64_t elapsedMs) const
{
    double dx = static_cast<double>(x - centerX_);
    double dy = static_cast<double>(y - centerY_);

    // atan2(0,0) returns 0 and hypot(0,0) returns 0, so the center needs no
    // special case; the distance term then contributes 0 to the hue.
    double angleDeg = std::atan2(dy, dx) * (180.0 / kPi);
    double distance = std::hypot(dx, dy);

    // Keep fractional hue; integer rounding produced visible color steps.
    const double raw = angleDeg + distance * tightnessDegPerPx_ + (elapsedMs % periodMs_) * 360.0 / periodMs_;
    const double hue = std::fmod(std::fmod(raw,360.0)+360.0,360.0);

    return HsvFullToRgb(hue);
}

void SpiralEffect::setReferencePoint(int x, int y)
{
    centerX_ = x;
    centerY_ = y;
}

std::pair<int,int> SpiralEffect::referencePoint() const
{
    return {centerX_, centerY_};
}
