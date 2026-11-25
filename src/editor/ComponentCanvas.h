#pragma once
#include "ComponentDraft.h"
#include <QGraphicsView>
#include <optional>

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
    QSet<int> selectedRows() const;
    void selectRows(const QSet<int>& rows);
    void setGridVisible(bool visible);
    void setPathVisible(bool visible);
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
    QGraphicsItem* ledAt(QPoint viewportPosition) const;
    QPointF snapped(QPointF scenePoint, Qt::KeyboardModifiers modifiers) const;
    ComponentDraft draft_;
    Tool tool_ = Tool::Select;
    bool locked_ = false, grid_ = true, path_ = true;
    QVector<int> wireOrder_;
    QPoint pressPosition_, lastPan_;
    bool pressOnLed_ = false, panning_ = false, spaceHeld_ = false;
    QHash<int, QPointF> pressItemPositions_;
};
