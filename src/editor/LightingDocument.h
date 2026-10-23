#pragma once

#include "components/ComponentCatalog.h"
#include <QColor>
#include <QImage>
#include <QObject>
#include <QUndoStack>
#include <QUuid>
#include <functional>

enum class LayerBlend { Normal, Multiply, Screen, Overlay };
struct EffectLayer {
    QUuid id = QUuid::createUuid();
    QString name = "Static Color";
    QColor color = Qt::white;
    bool visible = true;
    int transparency = 0;
    LayerBlend blend = LayerBlend::Normal;
    QImage mask;
    bool maskEnabled = true;
};
struct ComponentPlacement {
    QUuid id = QUuid::createUuid();
    QString componentId;
    QPointF origin{80, 80};
};
struct LightingState {
    QVector<ComponentPlacement> placements;
    QVector<EffectLayer> layers; // Bottom to top; panel presentation is reversed.
};

// Offline document only. Catalog geometry is never inferred as attached hardware.
class LightingDocument : public QObject {
    Q_OBJECT
public:
    explicit LightingDocument(ComponentCatalog catalog, QObject* parent = nullptr);
    static QSize canvasSize() { return {640, 400}; }
    const ComponentCatalog& catalog() const { return catalog_; }
    const LightingState& state() const { return state_; }
    QUndoStack* history() { return &history_; }
    void edit(const QString& label, const std::function<void(LightingState&)>& operation);
    QUuid addLayer();
    QUuid addComponent(const QString& componentId);
    QImage render() const;
    static QImage render(const LightingState& state);
    static QImage whiteMask();
    void save(const QString& fileName);
    void load(const QString& fileName);
    static void paintMask(QImage& mask, QPointF from, QPointF to,
        double radius, double hardness, double opacity, int gray);
    static void gradientMask(QImage& mask, QPointF from, QPointF to);
signals:
    void changed();
private:
    ComponentCatalog catalog_;
    LightingState state_;
    QUndoStack history_;
};
