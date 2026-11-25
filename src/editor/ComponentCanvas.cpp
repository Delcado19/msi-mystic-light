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
        // Do not blow a handful of LEDs up to the whole canvas.
        if (zoom() > 4) { const auto center = padded.center(); resetTransform(); scale(4, 4); centerOn(center); }
    } else centerOn(0, 0);
    emit zoomChanged(zoom());
}

void ComponentCanvas::rebuild()
{
    const auto selection = selectedRows();
    scene()->clear();
    const bool wiring = tool_ == Tool::Wire;
    QMap<int, QPointF> firstOfAddress;
    for (int row = 0; row < draft_.positions.size(); ++row) {
        const int address = draft_.mapping.value(row);
        if (draft_.nonPhysicalSlots.contains(address)) continue;
        const auto point = draft_.positions[row] * unit;
        const int pending = wiring ? int(wireOrder_.indexOf(address)) : -1;
        auto* led = new LedItem(-ledRadius, -ledRadius, 2 * ledRadius, 2 * ledRadius);
        led->setBrush(pending >= 0 ? studioThemeColor("@accent", 150) : studioThemeColor("@selected"));
        led->setData(0, row); led->setData(1, address);
        led->setPos(point);
        led->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIgnoresTransformations);
        led->setFlag(QGraphicsItem::ItemIsMovable, !wiring);
        led->setToolTip(QString("LED %1 · %2, %3").arg(address + 1).arg(draft_.positions[row].x()).arg(draft_.positions[row].y()));
        led->setZValue(1);
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
            if (firstOfAddress.contains(address)) {
                QPen shared(studioThemeColor("@secondary", 170), 1.5, Qt::DotLine); shared.setCosmetic(true);
                scene()->addLine(QLineF(firstOfAddress.value(address), point), shared)->setZValue(0);
            } else firstOfAddress.insert(address, point);
        }
    }
    if (path_) {
        QPen pathPen(studioThemeColor("@accent", 170), 1.5, Qt::DashLine); pathPen.setCosmetic(true);
        const auto order = wiring ? wireOrder_ : firstOfAddress.keys().toVector();
        for (int i = 1; i < order.size(); ++i)
            scene()->addLine(QLineF(firstOfAddress.value(order[i - 1]), firstOfAddress.value(order[i])), pathPen)->setZValue(0);
    }
    // A generous margin keeps panning possible in every direction, like an endless canvas.
    const auto bounds = scene()->itemsBoundingRect();
    scene()->setSceneRect(bounds.adjusted(-2000, -2000, 2000, 2000));
    selectRows(selection);
    viewport()->update();
}

QGraphicsItem* ComponentCanvas::ledAt(QPoint position) const
{
    for (auto* item : items(position)) {
        auto* led = item->parentItem() ? item->parentItem() : item;
        if (led->type() == QGraphicsEllipseItem::Type) return led;
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
        if (auto* led = ledAt(position)) {
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
        try { emit draftEdited(withAdded(draft_, snapped(mapToScene(position), event->modifiers()) / unit), "Add LED"); }
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
        ComponentDefinition artwork;
        artwork.category = draft_.category; artwork.name = draft_.name; artwork.id = draft_.id;
        artwork.positions = draft_.positions; artwork.mapping = draft_.mapping; artwork.nonPhysicalSlots = draft_.nonPhysicalSlots;
        painter->save(); painter->scale(unit, unit); drawHardwareComponentArtwork(*painter, artwork); painter->restore();
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
