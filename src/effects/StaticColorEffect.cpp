#include "effects/StaticColorEffect.h"

StaticColorEffect::StaticColorEffect(MsiColor color) : color_(color) {}

MsiColor StaticColorEffect::colorAt(int /*x*/, int /*y*/, int64_t /*elapsedMs*/) const
{
    return color_;
}
