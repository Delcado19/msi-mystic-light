#include "ui/LightingCanvasWidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPen>

#include "effects/IPositionableEffect.h"

namespace {
// Mirrors Canvas::tick's own cadence (src/canvas/Canvas.h) so the on-screen
// preview advances at roughly the same rate the real hardware would.
constexpr int kRepaintIntervalMs = 1000 / 60;
// The background is sampled every few pixels rather than per-pixel: a
// per-pixel QPainter::fillRect loop over a few hundred thousand pixels at
// 60Hz is needless cost for a preview, and blocks this small are visually
// indistinguishable from a true per-pixel render at normal window sizes.
constexpr int kBackgroundSampleBlockPx = 4;
}

LightingCanvasWidget::LightingCanvasWidget(QWidget* parent)
    : QWidget(parent)
{
    clock_.start();
    connect(&repaintTimer_, &QTimer::timeout, this, [this] { update(); });
    repaintTimer_.start(kRepaintIntervalMs);
}

void LightingCanvasWidget::setChannel(const std::string& channelName, CanvasPlacement placement, CanvasSource source, std::shared_ptr<IEffect> ownEffect)
{
    channels_[channelName] = ChannelBox{placement, source, std::move(ownEffect)};
}

void LightingCanvasWidget::removeChannel(const std::string& channelName)
{
    channels_.erase(channelName);
    // The box being dragged just disappeared out from under the cursor;
    // drop the stale drag state rather than keep moving a box that no
    // longer exists in channels_.
    if (dragKind_ == DragTarget::Channel && draggedChannel_ == channelName) {
        dragKind_ = DragTarget::None;
    }
}

void LightingCanvasWidget::setGlobalEffect(std::shared_ptr<IEffect> effect)
{
    globalEffect_ = std::move(effect);
}

IPositionableEffect* LightingCanvasWidget::globalEffectPositionable() const
{
    return globalEffect_ ? dynamic_cast<IPositionableEffect*>(globalEffect_.get()) : nullptr;
}

void LightingCanvasWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.fillRect(rect(), QColor(24, 24, 28));

    const int64_t elapsedMs = clock_.elapsed();

    if (globalEffect_) {
        // ponytail: one QPainter::fillRect call per 4px block is ~60k
        // calls/frame at a 1200x800 window size, likely too slow to hold
        // 60Hz on a large window. The brief named fillRect per block as
        // the implementer's-call default, so kept as-is for v1; if this
        // turns out to be the bottleneck, upgrade to writing a quarter-res
        // QImage's scanlines directly (one buffer write per block instead
        // of a QPainter call) and a single scaled QPainter::drawImage.
        for (int y = 0; y < height(); y += kBackgroundSampleBlockPx) {
            for (int x = 0; x < width(); x += kBackgroundSampleBlockPx) {
                const MsiColor c = globalEffect_->colorAt(x, y, elapsedMs);
                painter.fillRect(QRect(x, y, kBackgroundSampleBlockPx, kBackgroundSampleBlockPx), QColor(c.r, c.g, c.b));
            }
        }
    }

    // Fixed case-outline backdrop: a tower rectangle above a keyboard/mouse
    // row, mirroring the user's physical desk layout confirmed via mockup
    // (docs/superpowers/specs/2026-10-19-lighting-studio-design.md,
    // "Layout"). Drawn once per frame from plain shapes -- no new asset
    // pipeline needed for this.
    painter.setPen(QPen(QColor(90, 90, 100), 2));
    painter.setBrush(Qt::NoBrush);
    const int towerBottom = height() * 2 / 3;
    painter.drawRoundedRect(QRect(20, 10, width() - 40, towerBottom - 20), 8, 8);
    painter.drawRoundedRect(QRect(20, towerBottom, width() - 40, height() - towerBottom - 10), 8, 8);

    for (const auto& [name, box] : channels_) {
        const QRect r(qRound(box.placement.x), qRound(box.placement.y), kBoxWidth, kBoxHeight);
        if (box.source == CanvasSource::Independent && box.ownEffect) {
            const MsiColor c = box.ownEffect->colorAt(0, 0, elapsedMs);
            painter.setBrush(QColor(c.r, c.g, c.b));
        } else {
            // GlobalFollow (or no own effect assigned yet): outline only,
            // so the live background effect shows through the box --
            // matches the mockup's visual language for "this region tracks
            // the shared canvas effect".
            painter.setBrush(Qt::NoBrush);
        }
        painter.setPen(QPen(Qt::white, 1));
        painter.drawRoundedRect(r, 4, 4);
        painter.drawText(r, Qt::AlignCenter, QString::fromStdString(name));
    }

    if (auto* positionable = globalEffectPositionable()) {
        const auto [rx, ry] = positionable->referencePoint();
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::white);
        painter.drawEllipse(QPoint(rx, ry), kHandleHitRadius / 2, kHandleHitRadius / 2);
    }
}

void LightingCanvasWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) return;

    const QPoint pos = event->position().toPoint();

    // Hit-test in reverse paint order (paintEvent draws the handle last,
    // on top of every box; see below): the handle is checked first, then
    // boxes from topmost-drawn to bottommost-drawn. Checking boxes first
    // in forward map order would let a box hidden under the handle (or
    // under another box) steal clicks meant for whatever is actually
    // visible on top -- e.g. a motherboard zone box sitting under a
    // radial effect's draggable center.
    if (auto* positionable = globalEffectPositionable()) {
        const auto [rx, ry] = positionable->referencePoint();
        if ((pos - QPoint(rx, ry)).manhattanLength() <= kHandleHitRadius) {
            dragKind_ = DragTarget::GlobalReferencePoint;
            return;
        }
    }

    for (auto it = channels_.rbegin(); it != channels_.rend(); ++it) {
        const auto& name = it->first;
        const auto& box = it->second;
        const QRect r(qRound(box.placement.x), qRound(box.placement.y), kBoxWidth, kBoxHeight);
        if (r.contains(pos)) {
            dragKind_ = DragTarget::Channel;
            draggedChannel_ = name;
            dragOffset_ = pos - r.topLeft();
            return;
        }
    }

    dragKind_ = DragTarget::None;
}

void LightingCanvasWidget::mouseMoveEvent(QMouseEvent* event)
{
    const QPoint pos = event->position().toPoint();

    if (dragKind_ == DragTarget::Channel) {
        auto it = channels_.find(draggedChannel_);
        if (it == channels_.end()) { dragKind_ = DragTarget::None; return; }
        const QPoint newTopLeft = pos - dragOffset_;
        // Qt keeps delivering move/release events past the widget's own
        // rect() while a button is held (including negative coordinates),
        // so an unclamped drag released past an edge would leave the box
        // positioned off-canvas with no way to click it again -- and that
        // off-canvas value is exactly what channelPlacementChanged persists.
        // Clamp so the box stays fully visible within the widget.
        const int maxX = qMax(0, width() - kBoxWidth);
        const int maxY = qMax(0, height() - kBoxHeight);
        const int clampedX = qBound(0, newTopLeft.x(), maxX);
        const int clampedY = qBound(0, newTopLeft.y(), maxY);
        it->second.placement = CanvasPlacement{static_cast<double>(clampedX), static_cast<double>(clampedY)};
        emit channelPlacementChanged(draggedChannel_, it->second.placement);
        update();
    } else if (dragKind_ == DragTarget::GlobalReferencePoint) {
        if (auto* positionable = globalEffectPositionable()) {
            // Same off-canvas-drag issue as the channel-box branch above,
            // but for the global effect's reference-point handle: clamp so
            // the handle (drawn with radius kHandleHitRadius/2) stays fully
            // visible and re-clickable afterward.
            const int handleRadius = kHandleHitRadius / 2;
            const int maxX = qMax(handleRadius, width() - handleRadius);
            const int maxY = qMax(handleRadius, height() - handleRadius);
            const int clampedX = qBound(handleRadius, pos.x(), maxX);
            const int clampedY = qBound(handleRadius, pos.y(), maxY);
            positionable->setReferencePoint(clampedX, clampedY);
            emit globalEffectReferencePointChanged(clampedX, clampedY);
            update();
        }
    }
}

void LightingCanvasWidget::mouseReleaseEvent(QMouseEvent*)
{
    if (dragKind_ == DragTarget::Channel) {
        auto it = channels_.find(draggedChannel_);
        if (it != channels_.end()) {
            // One more emission on release, per the public contract (see
            // header): lets callers persist the final placement without
            // having to debounce every live drag update themselves.
            emit channelPlacementChanged(draggedChannel_, it->second.placement);
        }
    }
    dragKind_ = DragTarget::None;
    draggedChannel_.clear();
}
