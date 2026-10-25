#pragma once

#include "LightingDocument.h"
#include <QWidget>
#include <optional>
#include <QElapsedTimer>

class QPushButton;
class QPainter;
class QScrollBar;
class QTimer;
class LightingCanvas : public QWidget {
    Q_OBJECT
public:
    enum class Context { Layout, Effects };
    enum class Tool { Move, Pencil, Line, Rectangle, Ellipse, Gradient, Fill, Eraser };
    Tool tool() const { return drawingTool_; }
    void setTool(Tool tool) { cancelGesture(); drawingTool_=tool; setCursor(tool==Tool::Move ? Qt::ArrowCursor : Qt::CrossCursor); }
    void setGradientStops(QVector<QColor> colors,QVector<double> positions) { gradientColors_=std::move(colors); gradientPositions_=std::move(positions); }
    void setDrawing(QColor color,double width,bool filled) { drawColor_=color; drawWidth_=width; filled_=filled; }
    void setPreviewOnly();
    enum class MaskTool { Brush, Gradient };
    LightingCanvas(LightingDocument& document, Context context, QWidget* parent = nullptr);
    Context context() const { return context_; }
    QUuid selection() const { return selected_; }
    void select(const QUuid& id);
    QVector<ComponentPlacement> previewPlacements() const;
    void setMaskTarget(bool enabled);
    void setMaskTool(MaskTool tool) { cancelGesture(); tool_ = tool; }
    void setBrush(double radius, double hardness, double opacity, int gray);
    void setShowMask(bool show) { showMask_ = show; update(); }
    void setCropTool(bool enabled) { cancelGesture(); cropTool_ = enabled; update(); }
    void setPreviewFrame(const QImage& frame, double seconds, const QUuid& channel = {});
    void fitToView();
    // Non-finite/non-positive factors leave the view unchanged; valid zoom clamps to .001..4.
    void zoomBy(double factor);
    void gridChanged() { cancelGesture(); update(); }
    // Pixels per second at the outer edge; bounded to keep malformed settings harmless.
    void setAutoScrollSpeed(double pixelsPerSecond);
    QRectF contentRect() const;
    QPointF viewportPoint(QPointF canvas) const;
    QPointF canvasPoint(QPointF viewport) const;
signals:
    void selectionChanged();
    void layoutPreviewChanged();
    void effectGestureStarted();
    void gestureEnded();
    void maskEditingRequested();
    void editingError(const QString& message);
    void addRequested();
    void fullscreenRequested();
protected:
    void resizeEvent(QResizeEvent*) override;
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void hideEvent(QHideEvent*) override;
private:
    bool selectedPlacement(const QUuid& id) const;
    bool visibleChain(const ChannelAssignment& assignment) const;
    QRectF groupBounds(const ChannelAssignment& assignment) const;
    QRectF groupLabel(const ChannelAssignment& assignment) const;
    QRectF selectionBounds() const;
    QRectF componentBounds(const ComponentPlacement& placement) const;
    QRectF componentLabel(const ComponentPlacement& placement) const;
    ComponentPlacement previewPlacement(ComponentPlacement placement) const;
    void cancelGesture();
    void beginDrawing(QPointF point);
    void updateDrawing(QPointF point);
    QPointF paintPoint(QPointF point) const;
    void updateEffectGesture(QPointF point);
    QPointF offset() const;
    QSizeF displayScale() const;
    QVector<QPointF> componentPoints(const ComponentPlacement& placement) const;
    LightingDocument& document_;
    Context context_;
    QImage rendered_, workingMask_;
    QPushButton* emptyAction_;
    QPushButton* fullscreen_;
    QScrollBar *horizontalScroll_, *verticalScroll_;
    void syncScrollbars();
    QSizeF scrollUnits_{1,1};
    QTimer* autoScroll_;
    QElapsedTimer autoScrollClock_;
    QPointF pointer_, layoutAnchor_;
    Qt::KeyboardModifiers pointerModifiers_{};
    double autoScrollSpeed_=720;
    void scrollAtEdge();
    void paintMinimap(QPainter& painter);
    QRectF navigationBounds() const;
    QRectF minimapRect() const;
    void navigateMinimap(QPointF position);
    bool navigating_=false;
    bool autoFit_ = true, previewOnly_=false, drawing_=false, filled_=true;
    Tool drawingTool_=Tool::Move;
    QVector<QColor> gradientColors_; QVector<double> gradientPositions_;
    QColor drawColor_=Qt::white;
    double drawWidth_=6;
    QImage drawingBase_, fillReference_;
    QRectF previewBounds_{0,0,640,400};
    QUuid selected_, previewChannel_;
    bool maskTarget_ = false, showMask_ = false, dragging_ = false, panning_ = false;
    MaskTool tool_ = MaskTool::Brush;
    QPointF start_, last_, origin_, pan_;
    double zoom_ = 1, radius_ = 30, hardness_ = 0.3, opacity_ = 1;
    double seconds_ = 0;
    bool cropTool_ = false;
    int handle_ = 0;
    QRectF layoutBounds_;
    double layoutScale_=1, layoutRotation_=0;
    std::optional<EffectLayer> originalEffect_, workingEffect_;
    int gray_ = 0;
    bool snapGesture_ = false;
};
