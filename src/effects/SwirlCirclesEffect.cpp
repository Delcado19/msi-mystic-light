#include "effects/SwirlCirclesEffect.h"

#include <cmath>

#include "effects/HsvToRgb.h"

// Clamp to >=1: ringWidthPx_ is a divisor in colorAt, and periodMs_ is a
// divisor too (same reason as RadialRainbowEffect's constructor).
SwirlCirclesEffect::SwirlCirclesEffect(int centerX, int centerY, int ringWidthPx, int64_t periodMs)
    : centerX_(centerX), centerY_(centerY), ringWidthPx_(ringWidthPx > 0 ? ringWidthPx : 1), periodMs_(periodMs > 0 ? periodMs : 1)
{
}

MsiColor SwirlCirclesEffect::colorAt(int x, int y, int64_t elapsedMs) const
{
    double dx = static_cast<double>(x - centerX_);
    double dy = static_cast<double>(y - centerY_);
    double distance = std::hypot(dx, dy);

    // Continuous distance preserves ring motion without hard hue bands.
    const double raw = distance / ringWidthPx_ * 60.0 + (elapsedMs % periodMs_) * 360.0 / periodMs_;
    const double hue = std::fmod(std::fmod(raw,360.0)+360.0,360.0);

    return HsvFullToRgb(hue);
}

void SwirlCirclesEffect::setReferencePoint(int x, int y)
{
    centerX_ = x;
    centerY_ = y;
}

std::pair<int,int> SwirlCirclesEffect::referencePoint() const
{
    return {centerX_, centerY_};
}
