#pragma once
#include "ComponentDraft.h"
#include <QGraphicsView>
#include <optional>

// Wiring path runs (owner request 2026-11-25): one entry per edge between consecutive points, the run
// index (>= 0) or -1 for a jump. A jump changes layer or is longer than 2.5x the median edge
// (length jumps need at least 3 edges).
QVector<int> pathRuns(const QVector<QPointF>& points, const QVector<int>& layers);

// Component Editor drawing surface. It never owns undo state: every finished gesture is reported once
// through draftEdited(), and the page decides whether to accept it (setDraft) and record it.
class ComponentCanvas : public QGraphicsView {
    Q_OBJECT
public:
    enum class Tool { Select, Wire };
    static constexpr double unit = 24; // scene pixels per component unit at 100 % zoom
    explicit ComponentCanvas(QWidget* parent = nullptr);
    void setDraft(const ComponentDraft& draft); // redraw only, no signal
    const ComponentDraft& draft() const { return draft_; }
    void setTool(Tool tool);
    Tool tool() const { return tool_; }
    void setLocked(bool countAndWiringLocked); // instance mode: no add/delete, no Wire tool
    void setReadOnly(bool readOnly); // built-in components: view only, LEDs cannot be added, moved or deleted
    // Snap and dot spacing in component units, shared by snapping and the dot grid (owner review 2026-11-26).
    static constexpr double gridStep = 0.25;
    QSet<int> selectedRows() const;
    void selectRows(const QSet<int>& rows);
    // Align / distribute the selected LEDs (keys as LightingCanvas::alignSelection: left, center, right,
    // top, middle, bottom, distribute-horizontal, distribute-vertical); one "Align LEDs" edit. Returns the
    // reason when nothing happened, empty on success (owner request 2026-11-27).
    QString alignSelection(const QString& alignment);
    void setGridVisible(bool visible);
    void setPathVisible(bool visible);
    // Display-only depth layers: other layers are hidden, or dimmed and locked when all are visible.
    void setActiveLayer(int layer);
    int activeLayer() const { return activeLayer_; }
    void setAllLayersVisible(bool visible);
    bool allLayersVisible() const { return allLayers_; }
    void zoomBy(double factor);
    void fit();
    double zoom() const;
signals:
    void draftEdited(const ComponentDraft& after, const QString& label);
    void wireOrderFinished(const QVector<int>& addressOrder);
    void cursorMoved(std::optional<QPointF> componentPosition);
    void zoomChanged(double zoom);
protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void drawBackground(QPainter* painter, const QRectF& rect) override;
private:
    void rebuild();
    // Inactive-layer LEDs are only found for wiring, so deletes never reach a dimmed LED.
    QGraphicsItem* ledAt(QPoint viewportPosition, bool includeInactive = false) const;
    QPointF snapped(QPointF scenePoint, Qt::KeyboardModifiers modifiers) const;
    ComponentDraft draft_;
    Tool tool_ = Tool::Select;
    bool locked_ = false, readOnly_ = false, grid_ = true, path_ = true, allLayers_ = false;
    int activeLayer_ = 0;
    QVector<int> wireOrder_;
    QPoint pressPosition_, lastPan_;
    bool pressOnLed_ = false, panning_ = false, spaceHeld_ = false;
    QGraphicsItem* pressHandle_ = nullptr; QPointF pressHandlePosition_; // housing move/scale handle
    bool housingEditable() const;
    QPointF insideHousing(QPointF point, Qt::KeyboardModifiers modifiers) const; // component units
    QHash<int, QPointF> pressItemPositions_;
};
