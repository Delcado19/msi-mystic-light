#include "LightingCanvas.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QLineF>
#include <algorithm>
#include <cmath>

LightingCanvas::LightingCanvas(LightingDocument& document, Context context, QWidget* parent)
    : QWidget(parent), document_(document), context_(context)
{
    setMinimumSize(400, 280);
    setFocusPolicy(Qt::StrongFocus);
    setAccessibleName(context == Context::Layout ? "Component layout canvas" : "Effect and layer mask canvas");
    connect(&document_, &LightingDocument::changed, this, [this] {
        cancelGesture();
        rendered_ = document_.render();
        update();
    });
    rendered_ = document_.render();
}

QPointF LightingCanvas::offset() const
{
    return QPointF((width() - 640 * zoom_) / 2, (height() - 400 * zoom_) / 2) + pan_;
}
QPointF LightingCanvas::canvasPoint(QPointF viewport) const { return (viewport - offset()) / zoom_; }
void LightingCanvas::select(const QUuid& id)
{
    cancelGesture();
    selected_ = id;
    emit selectionChanged();
    update();
}
void LightingCanvas::setMaskTarget(bool enabled) { cancelGesture(); maskTarget_ = enabled; update(); }
void LightingCanvas::setBrush(double radius, double hardness, double opacity, int gray)
{
    radius_ = radius; hardness_ = hardness; opacity_ = opacity; gray_ = gray;
}
void LightingCanvas::cancelGesture()
{
    dragging_ = false;
    panning_ = false;
    workingMask_ = {};
    update();
}

QVector<QPointF> LightingCanvas::componentPoints(const ComponentPlacement& placement) const
{
    const auto& definition = document_.catalog()[placement.componentId];
    QVector<QPointF> points;
    for (const auto& point : definition.positions) points.append(point * 20 + placement.origin);
    return points;
}

void LightingCanvas::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.fillRect(rect(), QColor("#202329"));
    painter.translate(offset());
    painter.scale(zoom_, zoom_);
    painter.setClipRect(QRect(QPoint(), LightingDocument::canvasSize()));
    for (int y = 0; y < 400; y += 16)
        for (int x = 0; x < 640; x += 16)
            painter.fillRect(x, y, 16, 16, (x / 16 + y / 16) % 2 ? QColor("#484b50") : QColor("#35383d"));
    auto frame = rendered_;
    if (context_ == Context::Effects) {
        if (!workingMask_.isNull()) {
            auto state = document_.state();
            for (auto& layer : state.layers) if (layer.id == selected_) layer.mask = workingMask_;
            frame = LightingDocument::render(state);
        }
        if (showMask_ && maskTarget_) {
            for (const auto& layer : document_.state().layers)
                if (layer.id == selected_) frame = workingMask_.isNull() ? layer.mask : workingMask_;
        }
        painter.drawImage(0, 0, frame);
        if (dragging_ && tool_ == MaskTool::Gradient) {
            painter.setPen(QPen(Qt::yellow, 1 / zoom_));
            painter.drawLine(start_, last_);
        }
        return;
    }
    // Layout previews the current effect, but only physical placements are editable.
    painter.drawImage(0, 0, frame);
    painter.setRenderHint(QPainter::Antialiasing);
    for (auto placement : document_.state().placements) {
        if (dragging_ && placement.id == selected_) placement.origin = origin_ + last_ - start_;
        const auto points = componentPoints(placement);
        const auto& definition = document_.catalog()[placement.componentId];
        painter.setPen(QPen(placement.id == selected_ ? Qt::yellow : Qt::white, 1.5 / zoom_));
        for (int i = 0; i < points.size(); ++i) {
            const auto point = points[i];
            if (!QRectF(frame.rect()).adjusted(-20, -20, 20, 20).contains(point)) continue;
            const QColor light = QRectF(frame.rect()).contains(point) ? frame.pixelColor(point.toPoint()) : Qt::black;
            painter.setBrush(light);
            painter.drawEllipse(point, 6, 6);
            painter.drawText(point + QPointF(7, -7), QString::number(definition.mapping[i]));
        }
        if (QRectF(frame.rect()).contains(placement.origin))
            painter.drawText(placement.origin + QPointF(0, -22), definition.name);
    }
}

void LightingCanvas::mousePressEvent(QMouseEvent* event)
{
    setFocus();
    if (event->button() == Qt::MiddleButton) { cancelGesture(); panning_ = true; last_ = event->position(); return; }
    if (event->button() != Qt::LeftButton) return;
    start_ = last_ = canvasPoint(event->position());
    if (!QRectF(QPointF(), LightingDocument::canvasSize()).contains(start_)) return;
    if (context_ == Context::Layout) {
        QUuid hit;
        for (auto it = document_.state().placements.crbegin(); it != document_.state().placements.crend(); ++it) {
            for (auto point : componentPoints(*it))
                if (QLineF(point, start_).length() < 12 / zoom_) { hit = it->id; origin_ = it->origin; break; }
            if (!hit.isNull()) break;
        }
        select(hit);
        dragging_ = !hit.isNull();
    } else if (maskTarget_) {
        for (const auto& layer : document_.state().layers) if (layer.id == selected_) {
            workingMask_ = layer.mask;
            dragging_ = true;
            if (tool_ == MaskTool::Brush)
                LightingDocument::paintMask(workingMask_, start_, start_, radius_, hardness_, opacity_, gray_);
        }
    }
    update();
}

void LightingCanvas::mouseMoveEvent(QMouseEvent* event)
{
    if (panning_) { pan_ += event->position() - last_; last_ = event->position(); update(); return; }
    if (!dragging_) return;
    const auto point = canvasPoint(event->position());
    if (context_ == Context::Effects && tool_ == MaskTool::Brush)
        LightingDocument::paintMask(workingMask_, last_, point, radius_, hardness_, opacity_, gray_);
    last_ = point;
    if (context_ == Context::Effects && tool_ == MaskTool::Gradient) {
        for (const auto& layer : document_.state().layers)
            if (layer.id == selected_) workingMask_ = layer.mask;
        LightingDocument::gradientMask(workingMask_, start_, last_);
    }
    update();
}

void LightingCanvas::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton) { panning_ = false; return; }
    if (event->button() != Qt::LeftButton || !dragging_) return;
    const auto id = selected_;
    if (context_ == Context::Layout) {
        const auto target = origin_ + last_ - start_;
        if (last_ != start_) document_.edit("Move component", [&](LightingState& state) {
            for (auto& placement : state.placements) if (placement.id == id) placement.origin = target;
        });
    } else {
        const auto mask = workingMask_;
        for (const auto& layer : document_.state().layers) if (layer.id == id && layer.mask != mask) {
            document_.edit("Edit layer mask", [&](LightingState& state) {
                for (auto& changed : state.layers) if (changed.id == id) changed.mask = mask;
            });
            break;
        }
    }
    cancelGesture();
}

void LightingCanvas::wheelEvent(QWheelEvent* event)
{
    cancelGesture();
    const auto before = canvasPoint(event->position());
    zoom_ = std::clamp(zoom_ * std::pow(1.15, event->angleDelta().y() / 120.0), 0.25, 4.0);
    pan_ += event->position() - (before * zoom_ + offset());
    update();
}

void LightingCanvas::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) { cancelGesture(); return; }
    if (context_ == Context::Layout && !selected_.isNull()) {
        QPointF delta;
        if (event->key() == Qt::Key_Left) delta.setX(-1);
        if (event->key() == Qt::Key_Right) delta.setX(1);
        if (event->key() == Qt::Key_Up) delta.setY(-1);
        if (event->key() == Qt::Key_Down) delta.setY(1);
        if (!delta.isNull()) {
            if (event->modifiers() & Qt::ShiftModifier) delta *= 10;
            document_.edit("Nudge component", [&](LightingState& state) {
                for (auto& placement : state.placements) if (placement.id == selected_) placement.origin += delta;
            });
            return;
        }
    }
    QWidget::keyPressEvent(event);
}
void LightingCanvas::hideEvent(QHideEvent* event) { cancelGesture(); QWidget::hideEvent(event); }
