// A solid, unchanging color across the whole surface.
#pragma once

#include "effects/IEffect.h"

class StaticColorEffect : public IEffect {
public:
    explicit StaticColorEffect(MsiColor color);
    MsiColor colorAt(int x, int y, int64_t elapsedMs) const override; // ignores inputs, returns the constructor color

private:
    MsiColor color_;
};
