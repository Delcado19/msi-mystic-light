#include "ComponentCanvas.h"
#include "HardwareArtwork.h"
#include "SnapGrid.h"
#include "StudioTheme.h"
#include <QApplication>
#include <QGraphicsEllipseItem>
#include <QGraphicsSimpleTextItem>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QStyleOptionGraphicsItem>
#include <QWheelEvent>
#include <QGraphicsPolygonItem>
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {
constexpr double ledRadius = 12;  // screen pixels; markers ignore zoom so they stay clickable
constexpr double minZoom = 0.05, maxZoom = 20;

// Selection is shown by the accent ring instead of Qt's dashed bounding box.
class LedItem : public QGraphicsEllipseItem {
public:
    using QGraphicsEllipseItem::QGraphicsEllipseItem;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override {
        QStyleOptionGraphicsItem plain(*option);
        plain.state &= ~QStyle::State_Selected;
        if (isSelected()) setPen(QPen(studioThemeColor("@accent"), 3));
        else setPen(QPen(studioThemeColor("@border"), 1.5));
        QGraphicsEllipseItem::paint(painter, &plain, widget);
    }
};
}

QVector<int> pathRuns(const QVector<QPointF>& points, const QVector<int>& layers)
{
    QVector<double> lengths;
    for (int i = 1; i < points.size(); ++i) lengths.append(QLineF(points[i - 1], points[i]).length());
    auto sorted = lengths; std::sort(sorted.begin(), sorted.end());
    const double limit = sorted.size() >= 3 ? 2.5 * sorted[sorted.size() / 2] : -1;
    QVector<int> runs; int run = 0;
    for (int i = 0; i < lengths.size(); ++i) {
        const bool jump = layers.value(i, 0) != layers.value(i + 1, 0) || (limit > 0 && lengths[i] > limit);
        if (jump) { runs.append(-1); if (i > 0 && runs[i - 1] != -1) ++run; }
        else runs.append(run);
    }
    return runs;
}

ComponentCanvas::ComponentCanvas(QWidget* parent) : QGraphicsView(parent)
{
    setObjectName("componentCanvas");
    setScene(new QGraphicsScene(this));
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(QGraphicsView::NoAnchor);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setFrameShape(QFrame::NoFrame);
}

void ComponentCanvas::setDraft(const ComponentDraft& draft)
{
    const bool sameCount = draft.positions.size() == draft_.positions.size();
    const auto selection = sameCount ? selectedRows() : QSet<int>{};
    draft_ = draft;
    wireOrder_.clear();
    rebuild();
    selectRows(selection);
}

void ComponentCanvas::setTool(Tool tool)
{
    if (locked_ && tool == Tool::Wire) tool = Tool::Select;
    tool_ = tool;
    wireOrder_.clear();
    rebuild();
}

void ComponentCanvas::setLocked(bool locked)
{
    locked_ = locked;
    if (locked_ && tool_ == Tool::Wire) setTool(Tool::Select);
}

QSet<int> ComponentCanvas::selectedRows() const
{
    QSet<int> rows;
    for (auto* item : scene()->selectedItems()) rows.insert(item->data(0).toInt());
    return rows;
}

void ComponentCanvas::selectRows(const QSet<int>& rows)
{
    for (auto* item : scene()->items())
        if (item->type() == QGraphicsEllipseItem::Type) item->setSelected(rows.contains(item->data(0).toInt()));
}

void ComponentCanvas::setGridVisible(bool visible) { grid_ = visible; viewport()->update(); }
void ComponentCanvas::setPathVisible(bool visible) { path_ = visible; rebuild(); }
void ComponentCanvas::setActiveLayer(int layer)
{
    const auto selection = selectedRows();
    activeLayer_ = std::max(0, layer); rebuild(); selectRows(selection);
}
void ComponentCanvas::setAllLayersVisible(bool visible)
{
    const auto selection = selectedRows();
    allLayers_ = visible; rebuild(); selectRows(selection);
}
double ComponentCanvas::zoom() const { return transform().m11(); }

void ComponentCanvas::zoomBy(double factor)
{
    const double target = std::clamp(zoom() * factor, minZoom, maxZoom);
    const auto center = mapToScene(viewport()->rect().center());
    scale(target / zoom(), target / zoom());
    centerOn(center);
    emit zoomChanged(zoom());
}

void ComponentCanvas::fit()
{
    QRectF bounds;
    for (auto* item : scene()->items()) if (item->type() == QGraphicsEllipseItem::Type)
        bounds = bounds.isNull() ? QRectF(item->pos(), QSizeF()) : bounds.united(QRectF(item->pos(), QSizeF()));
    resetTransform();
    if (!bounds.isNull()) {
        const auto padded = bounds.adjusted(-3 * ledRadius, -3 * ledRadius, 3 * ledRadius, 3 * ledRadius);
        fitInView(padded, Qt::KeepAspectRatio);
        // Do not magnify a single LED until it fills the canvas. Dense layouts (keyboards: 0.25 units
        // between keys) need the full fit zoom so markers stay apart.
        if (bounds.width() == 0 && bounds.height() == 0 && zoom() > 2) {
            const auto center = padded.center(); resetTransform(); scale(2, 2); centerOn(center);
        }
    } else centerOn(0, 0);
    emit zoomChanged(zoom());
}

void ComponentCanvas::rebuild()
{
    const auto selection = selectedRows();
    scene()->clear();
    const bool wiring = tool_ == Tool::Wire;
    // The wire path is computed over every layer, so a hidden layer never joins two visible LEDs with a
    // false edge (owner report 2026-11-25: SWAFAN 12 -> 25). Only edges with both ends visible are drawn.
    QMap<int, QPointF> firstOfAddress, visibleFirst;
    QMap<int, int> layerOfAddress;
    for (int row = 0; row < draft_.positions.size(); ++row) {
        const int address = draft_.mapping.value(row);
        if (draft_.nonPhysicalSlots.contains(address) || firstOfAddress.contains(address)) continue;
        firstOfAddress.insert(address, draft_.positions[row] * unit); layerOfAddress.insert(address, draft_.layers.value(row, 0));
    }
    const bool layered = draft_.layerCount() > 1;
    for (int row = 0; row < draft_.positions.size(); ++row) {
        const int address = draft_.mapping.value(row);
        if (draft_.nonPhysicalSlots.contains(address)) continue;
        const int layer = draft_.layers.value(row, 0);
        const bool active = layer == activeLayer_;
        if (!active && !allLayers_) continue;
        const auto point = draft_.positions[row] * unit;
        const int pending = wiring ? int(wireOrder_.indexOf(address)) : -1;
        auto* led = new LedItem(-ledRadius, -ledRadius, 2 * ledRadius, 2 * ledRadius);
        led->setBrush(pending >= 0 ? studioThemeColor("@accent", 150) : studioThemeColor("@selected"));
        led->setData(0, row); led->setData(1, address); led->setData(2, active);
        led->setPos(point);
        led->setFlags(active ? QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIgnoresTransformations : QGraphicsItem::ItemIgnoresTransformations);
        led->setFlag(QGraphicsItem::ItemIsMovable, active && !wiring);
        led->setToolTip(QString("LED %1 · %2, %3").arg(address + 1).arg(draft_.positions[row].x()).arg(draft_.positions[row].y())
            + (layered ? QString(" · layer %1").arg(layer + 1) : QString()));
        led->setZValue(active ? 2 : 1);
        if (!active) led->setOpacity(0.35);
        scene()->addItem(led);
        const auto label = wiring ? (pending >= 0 ? QString::number(pending + 1) : QString()) : QString::number(address + 1);
        auto* text = new QGraphicsSimpleTextItem(label, led);
        text->setBrush(studioThemeColor("@text"));
        QFont font = text->font(); font.setPixelSize(11); font.setWeight(QFont::DemiBold); text->setFont(font);
        text->setScale(qMin(1.0, 20.0 / qMax(1.0, text->boundingRect().width())));
        const auto size = text->boundingRect().size() * text->scale();
        text->setPos(-size.width() / 2, -size.height() / 2);
        text->setAcceptedMouseButtons(Qt::NoButton);
        if (path_) {
            if (visibleFirst.contains(address)) {
                QPen shared(studioThemeColor("@secondary", 170), 1.5, Qt::DotLine); shared.setCosmetic(true);
                scene()->addLine(QLineF(visibleFirst.value(address), point), shared)->setZValue(0);
            } else visibleFirst.insert(address, point);
        }
    }
    if (path_) {
        // One color per run, dashed arrows for jumps between runs (owner request 2026-11-25). The theme has
        // a single hue, so the run palette is local.
        static const QColor palette[] = {QColor("#45DDF5"), QColor("#F5B945"), QColor("#E56BF0"), QColor("#7EE06A")};
        auto order = wiring ? wireOrder_ : firstOfAddress.keys().toVector();
        order.erase(std::remove_if(order.begin(), order.end(), [&](int address) { return !firstOfAddress.contains(address); }), order.end());
        QVector<QPointF> points; QVector<int> layers;
        for (const int address : order) { points.append(firstOfAddress.value(address)); layers.append(layerOfAddress.value(address)); }
        const auto runs = pathRuns(points, layers);
        for (int i = 0; i < runs.size(); ++i) {
            if (visibleFirst.value(order[i]) != points[i] || visibleFirst.value(order[i + 1]) != points[i + 1]
                || !visibleFirst.contains(order[i]) || !visibleFirst.contains(order[i + 1])) continue;
            const QLineF edge(points[i], points[i + 1]);
            QColor color = runs[i] < 0 ? studioThemeColor("@text", 150) : palette[runs[i] % 4];
            if (runs[i] >= 0) color.setAlpha(200);
            QPen pen(color, runs[i] < 0 ? 1.5 : 2, runs[i] < 0 ? Qt::DashLine : Qt::SolidLine); pen.setCosmetic(true);
            scene()->addLine(edge, pen)->setZValue(0);
            if (runs[i] < 0 && edge.length() > 0) {
                // Arrowhead in screen pixels, just outside the target LED marker.
                auto* head = scene()->addPolygon(QPolygonF{QPointF(-ledRadius, 0), QPointF(-ledRadius - 9, -4.5), QPointF(-ledRadius - 9, 4.5)}, Qt::NoPen, color);
                head->setFlag(QGraphicsItem::ItemIgnoresTransformations);
                head->setPos(edge.p2()); head->setRotation(-edge.angle());
                head->setZValue(0);
            }
        }
    }
    // A generous margin keeps panning possible in every direction, like an endless canvas.
    const auto bounds = scene()->itemsBoundingRect();
    scene()->setSceneRect(bounds.adjusted(-2000, -2000, 2000, 2000));
    selectRows(selection);
    viewport()->update();
}

QGraphicsItem* ComponentCanvas::ledAt(QPoint position, bool includeInactive) const
{
    for (auto* item : items(position)) {
        auto* led = item->parentItem() ? item->parentItem() : item;
        if (led->type() == QGraphicsEllipseItem::Type && (includeInactive || led->data(2).toBool())) return led;
    }
    return nullptr;
}

QPointF ComponentCanvas::snapped(QPointF point, Qt::KeyboardModifiers modifiers) const
{
    auto grid = SnapGrid::load();
    if (!grid.enabled || (modifiers & Qt::AltModifier)) return point;
    grid.spacing *= unit / 20; // grid spacing is in Layout pixels; one component unit is 20 Layout pixels
    return grid.snap(point);
}

void ComponentCanvas::mousePressEvent(QMouseEvent* event)
{
    const auto position = event->position().toPoint();
    if (event->button() == Qt::MiddleButton || (event->button() == Qt::LeftButton && spaceHeld_)) {
        panning_ = true; lastPan_ = position; viewport()->setCursor(Qt::ClosedHandCursor); event->accept(); return;
    }
    if (event->button() == Qt::RightButton) {
        if (auto* led = ledAt(position); led && !locked_) {
            try { emit draftEdited(withRemoved(draft_, {led->data(0).toInt()}), "Delete LED"); }
            catch (const std::invalid_argument&) {}
        }
        event->accept(); return;
    }
    if (event->button() != Qt::LeftButton) { QGraphicsView::mousePressEvent(event); return; }
    if (tool_ == Tool::Wire) {
        if (auto* led = ledAt(position, true)) {
            const int address = led->data(1).toInt();
            // A repeated click on a point in a shared zone counts only once.
            if (!wireOrder_.contains(address)) wireOrder_.append(address);
            const auto addresses = QSet<int>(draft_.mapping.cbegin(), draft_.mapping.cend()).size();
            if (wireOrder_.size() == addresses) {
                const auto order = wireOrder_;
                wireOrder_.clear();
                emit wireOrderFinished(order);
            }
            rebuild();
        }
        event->accept(); return;
    }
    pressPosition_ = position;
    pressOnLed_ = ledAt(position) != nullptr;
    setDragMode(pressOnLed_ ? QGraphicsView::NoDrag : QGraphicsView::RubberBandDrag);
    QGraphicsView::mousePressEvent(event);
    pressItemPositions_.clear();
    for (auto* item : scene()->selectedItems()) pressItemPositions_.insert(item->data(0).toInt(), item->pos());
}

void ComponentCanvas::mouseMoveEvent(QMouseEvent* event)
{
    const auto position = event->position().toPoint();
    if (panning_) {
        const auto delta = position - lastPan_; lastPan_ = position;
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        event->accept(); return;
    }
    emit cursorMoved(mapToScene(position) / unit);
    QGraphicsView::mouseMoveEvent(event);
}

void ComponentCanvas::mouseReleaseEvent(QMouseEvent* event)
{
    const auto position = event->position().toPoint();
    if (panning_ && (event->button() == Qt::MiddleButton || event->button() == Qt::LeftButton)) {
        panning_ = false; viewport()->setCursor(spaceHeld_ ? Qt::OpenHandCursor : Qt::ArrowCursor); event->accept(); return;
    }
    if (event->button() != Qt::LeftButton || tool_ == Tool::Wire) { QGraphicsView::mouseReleaseEvent(event); return; }
    QGraphicsView::mouseReleaseEvent(event);
    setDragMode(QGraphicsView::NoDrag);
    if (pressOnLed_) {
        pressOnLed_ = false;
        QList<QGraphicsItem*> moved;
        for (auto* item : scene()->selectedItems())
            if (pressItemPositions_.contains(item->data(0).toInt()) && pressItemPositions_.value(item->data(0).toInt()) != item->pos()) moved.append(item);
        if (moved.isEmpty()) return;
        // Snap the lowest moved row and carry the others along, so a shape keeps its form.
        auto* anchor = *std::min_element(moved.begin(), moved.end(), [](auto* a, auto* b) { return a->data(0).toInt() < b->data(0).toInt(); });
        const auto delta = snapped(anchor->pos(), event->modifiers()) - anchor->pos();
        auto after = draft_;
        for (auto* item : moved) after.positions[item->data(0).toInt()] = (item->pos() + delta) / unit;
        emit draftEdited(after, "Move LEDs");
        return;
    }
    if (!locked_ && (position - pressPosition_).manhattanLength() < QApplication::startDragDistance()) {
        try { emit draftEdited(withAdded(draft_, snapped(mapToScene(position), event->modifiers()) / unit, activeLayer_), "Add LED"); }
        catch (const std::invalid_argument&) {}
    }
}

void ComponentCanvas::wheelEvent(QWheelEvent* event)
{
    // Zoom at the cursor: the scene point under the pointer stays in place.
    const auto cursor = event->position().toPoint();
    const auto before = mapToScene(cursor);
    const double target = std::clamp(zoom() * std::pow(1.15, event->angleDelta().y() / 120.0), minZoom, maxZoom);
    scale(target / zoom(), target / zoom());
    const auto drift = mapFromScene(before) - cursor;
    horizontalScrollBar()->setValue(horizontalScrollBar()->value() + drift.x());
    verticalScrollBar()->setValue(verticalScrollBar()->value() + drift.y());
    emit zoomChanged(zoom());
    event->accept();
}

void ComponentCanvas::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
        spaceHeld_ = true; viewport()->setCursor(Qt::OpenHandCursor); event->accept(); return;
    }
    QGraphicsView::keyPressEvent(event);
}

void ComponentCanvas::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
        spaceHeld_ = false; viewport()->setCursor(Qt::ArrowCursor); event->accept(); return;
    }
    QGraphicsView::keyReleaseEvent(event);
}

void ComponentCanvas::leaveEvent(QEvent* event)
{
    emit cursorMoved(std::nullopt);
    QGraphicsView::leaveEvent(event);
}

void ComponentCanvas::drawBackground(QPainter* painter, const QRectF& rect)
{
    painter->fillRect(rect, studioThemeColor("@surround"));
    if (tool_ == Tool::Select && !draft_.positions.isEmpty()) {
        // Housing artwork as a placement reference; wiring gets a clean workplane.
        painter->save(); painter->scale(unit, unit); drawHardwareComponentArtwork(*painter, draft_.toDefinitionUnchecked()); painter->restore();
    }
    if (!grid_) return;
    // Dot grid on whole component units; skip dots until they are at least 16 screen pixels apart.
    double step = unit;
    while (step * zoom() < 16) step *= 2;
    painter->save();
    painter->setPen(QPen(studioThemeColor("@separator"), 2.5 / zoom(), Qt::SolidLine, Qt::RoundCap));
    int dots = 0;
    for (double x = std::floor(rect.left() / step) * step; x <= rect.right(); x += step)
        for (double y = std::floor(rect.top() / step) * step; y <= rect.bottom() && dots < 40000; y += step, ++dots)
            painter->drawPoint(QPointF(x, y));
    painter->restore();
}
