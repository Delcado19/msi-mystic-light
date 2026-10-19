// An effect with one spatial reference point the canvas lets the user drag
// (e.g. a radial gradient's center). Effects without such a parameter
// (Breathing, Marquee) implement only IEffect, not this. Later UI tasks
// (Task 14, LightingCanvasWidget) use this to let the user reposition the
// point by dragging on the canvas.
#pragma once

#include <utility>

class IPositionableEffect {
public:
    virtual ~IPositionableEffect() = default;
    virtual void setReferencePoint(int x, int y) = 0;
    virtual std::pair<int,int> referencePoint() const = 0;
};
