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

    // distance is >=0, so floor == truncation here; keep the explicit floor
    // to match the spec formula.
    int64_t ringIndex = static_cast<int64_t>(std::floor(distance / ringWidthPx_));

    // Same integer normalize-mod-360 idiom as SpiralEffect/RadialRainbowEffect.
    int64_t raw = ringIndex * 60 + (elapsedMs * 360) / periodMs_;
    int hue = static_cast<int>(((raw % 360) + 360) % 360);

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
