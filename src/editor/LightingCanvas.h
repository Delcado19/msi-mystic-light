#pragma once

#include "LightingDocument.h"
#include <QWidget>
#include <optional>

class QPushButton;
class LightingCanvas : public QWidget {
    Q_OBJECT
public:
    enum class Context { Layout, Effects };
    enum class Tool { Move, Pencil, Line, Rectangle, Ellipse, Gradient, Fill, Eraser };
    Tool tool() const { return drawingTool_; }
    void setTool(Tool tool) { cancelGesture(); drawingTool_=tool; setCursor(tool==Tool::Move ? Qt::ArrowCursor : Qt::CrossCursor); }
    void setDrawing(QColor color,double width,bool filled) { drawColor_=color; drawWidth_=width; filled_=filled; }
    void setPreviewOnly() { previewOnly_=true; setMinimumHeight(160); setFocusPolicy(Qt::NoFocus); }
    enum class MaskTool { Brush, Gradient };
    LightingCanvas(LightingDocument& document, Context context, QWidget* parent = nullptr);
    Context context() const { return context_; }
    QUuid selection() const { return selected_; }
    void select(const QUuid& id);
    void setMaskTarget(bool enabled);
    void setMaskTool(MaskTool tool) { cancelGesture(); tool_ = tool; }
    void setBrush(double radius, double hardness, double opacity, int gray);
    void setShowMask(bool show) { showMask_ = show; update(); }
    void setCropTool(bool enabled) { cancelGesture(); cropTool_ = enabled; update(); }
    void setPreviewFrame(const QImage& frame, double seconds, const QUuid& channel = {});
    void fitToView();
    QPointF viewportPoint(QPointF canvas) const;
    QPointF canvasPoint(QPointF viewport) const;
signals:
    void selectionChanged();
    void effectGestureStarted();
    void gestureEnded();
    void maskEditingRequested();
    void editingError(const QString& message);
    void addRequested();
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
    void cancelGesture();
    void beginDrawing(QPointF point);
    void updateDrawing(QPointF point);
    QPointF paintPoint(QPointF point) const;
    void updateEffectGesture(QPointF point);
    QPointF offset() const;
    QVector<QPointF> componentPoints(const ComponentPlacement& placement) const;
    LightingDocument& document_;
    Context context_;
    QImage rendered_, workingMask_;
    QPushButton* emptyAction_;
    bool autoFit_ = true, previewOnly_=false, drawing_=false, filled_=true;
    Tool drawingTool_=Tool::Move;
    QColor drawColor_=Qt::white;
    double drawWidth_=6;
    QImage drawingBase_;
    QRectF previewBounds_{0,0,640,400};
    QUuid selected_, previewChannel_;
    bool maskTarget_ = false, showMask_ = false, dragging_ = false, panning_ = false;
    MaskTool tool_ = MaskTool::Brush;
    QPointF start_, last_, origin_, pan_;
    double zoom_ = 1, radius_ = 30, hardness_ = 0.3, opacity_ = 1;
    double seconds_ = 0;
    bool cropTool_ = false;
    int handle_ = 0;
    std::optional<EffectLayer> originalEffect_, workingEffect_;
    int gray_ = 0;
};
