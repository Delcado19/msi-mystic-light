#pragma once

#include "LightingDocument.h"
#include <QWidget>

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
    void setPreviewTime(double seconds);
    QPointF canvasPoint(QPointF viewport) const;
signals:
    void selectionChanged();
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
    QPointF offset() const;
    QVector<QPointF> componentPoints(const ComponentPlacement& placement) const;
    LightingDocument& document_;
    Context context_;
    QImage rendered_, workingMask_;
    QUuid selected_;
    bool maskTarget_ = false, showMask_ = false, dragging_ = false, panning_ = false;
    MaskTool tool_ = MaskTool::Brush;
    QPointF start_, last_, origin_, pan_;
    double zoom_ = 1, radius_ = 30, hardness_ = 0.3, opacity_ = 1;
    double seconds_ = 0;
    int gray_ = 0;
};
