#pragma once

#include "LightingDocument.h"
#include <QWidget>
#include <optional>

class LightingCanvas : public QWidget {
    Q_OBJECT
public:
    enum class Context { Layout, Effects };
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
    QPointF canvasPoint(QPointF viewport) const;
signals:
    void selectionChanged();
    void effectGestureStarted();
protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void hideEvent(QHideEvent*) override;
private:
    void cancelGesture();
    void updateEffectGesture(QPointF point);
    QPointF offset() const;
    QVector<QPointF> componentPoints(const ComponentPlacement& placement) const;
    LightingDocument& document_;
    Context context_;
    QImage rendered_, workingMask_;
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
