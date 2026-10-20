// QWidget showing every channel (device) as a small draggable rectangle
// positioned over a live-rendered background: the canvas-wide global
// effect (if any), sampled across the widget's full area via QPainter and
// redrawn on a 60Hz QTimer (mirrors Canvas::tick's own rate -- see
// src/canvas/Canvas.h). A channel in GlobalFollow mode is drawn as an
// outline so the background effect shows through it; an Independent
// channel is filled with its own effect's color instead (Task 14 of the
// Lighting Studio plan; Task 15 embeds this widget into the main window).
// kBoxWidth/kBoxHeight below are the same box Canvas::tick() scales a
// GlobalFollow channel's real LED-coordinate extent into (see Canvas.h's
// kGlobalFollowBoxWidth/Height), so the box drawn/dragged here is always the
// same region actually sampled and pushed to that channel's hardware LEDs
// (Task 22 geometry fix).
#pragma once

#include <QElapsedTimer>
#include <QPoint>
#include <QTimer>
#include <QWidget>

#include <map>
#include <memory>
#include <string>

#include "canvas/Canvas.h" // CanvasPlacement, CanvasSource
#include "effects/IEffect.h"

class IPositionableEffect; // effects/IPositionableEffect.h -- only referenced by pointer here

class LightingCanvasWidget : public QWidget {
    Q_OBJECT
public:
    explicit LightingCanvasWidget(QWidget* parent = nullptr);

    // Registers (or replaces) one channel's canvas box; `name` is shown as
    // the box's label. Placement/source come from the ChannelSettings the
    // caller already owns -- this widget does not read Settings directly.
    void setChannel(const std::string& channelName, CanvasPlacement placement, CanvasSource source, std::shared_ptr<IEffect> ownEffect);
    void removeChannel(const std::string& channelName);
    void setGlobalEffect(std::shared_ptr<IEffect> effect);

    // Pixel size of a channel's drawn box, and the hit-test radius around
    // the global effect's draggable reference point. Exposed (rather than
    // kept as private magic numbers) so callers/tests can compute a box's
    // on-screen rect from the CanvasPlacement passed to setChannel without
    // duplicating these values. kBoxWidth/kBoxHeight alias Canvas.h's
    // kGlobalFollowBoxWidth/Height (Task 22 geometry fix) rather than
    // redeclaring their own 70/40 -- Canvas::tick() scales every GlobalFollow
    // channel's sampling into that exact same box, so what's drawn here and
    // what's actually sampled for the hardware must stay the same two
    // numbers, not just equal by coincidence.
    static constexpr int kBoxWidth = kGlobalFollowBoxWidth;
    static constexpr int kBoxHeight = kGlobalFollowBoxHeight;
    static constexpr int kHandleHitRadius = 10;

signals:
    // Emitted live while dragging a channel's box, and once more on
    // release. The widget already applies the new placement to its own
    // channels_ entry before emitting -- this signal is a notification for
    // the caller to persist (e.g. into Settings), not a request the
    // caller must apply back into the widget itself.
    void channelPlacementChanged(const std::string& channelName, CanvasPlacement newPlacement);
    // Emitted while dragging a positionable global effect's reference-point
    // handle. Like channelPlacementChanged, the widget has already called
    // setReferencePoint() on the (shared, mutated-in-place) global effect
    // object before emitting; this is a notification, not a request.
    void globalEffectReferencePointChanged(int x, int y);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;

private:
    struct ChannelBox { CanvasPlacement placement; CanvasSource source; std::shared_ptr<IEffect> ownEffect; };

    // Returns globalEffect_ re-exposed as IPositionableEffect, or nullptr
    // if unset or the assigned effect doesn't implement that interface
    // (see IPositionableEffect.h -- only some effects, e.g.
    // RadialRainbowEffect, expose a draggable reference point).
    IPositionableEffect* globalEffectPositionable() const;

    std::map<std::string, ChannelBox> channels_;
    std::shared_ptr<IEffect> globalEffect_;
    QTimer repaintTimer_;
    QElapsedTimer clock_;

    // Drag state: which box (by name) or "the global effect's reference
    // point" is currently grabbed, and the press-time offset, set in
    // mousePressEvent and read in mouseMoveEvent/mouseReleaseEvent.
    enum class DragTarget { None, Channel, GlobalReferencePoint };
    DragTarget dragKind_ = DragTarget::None;
    std::string draggedChannel_;
    QPoint dragOffset_; // press position minus the dragged box's top-left, so the box doesn't jump to re-center on the cursor
};
