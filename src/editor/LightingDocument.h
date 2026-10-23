#pragma once

#include "components/ComponentCatalog.h"
#include "EffectRenderer.h"
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
    EffectKind effect = EffectKind::StaticColor;
    QColor secondaryColor = Qt::black;
    QPointF center{320, 200};
    QSizeF extent{240, 240};
    double rotation = 0;
    int repetitions = 4;
    QImage image;
    bool imageFill = false; // Fit preserves the whole image; fill crops to the effect rectangle.
    double spin = 0, pulse = 0, hueShift = 0; // Degrees/second, cycles/second, degrees/second.
    bool visible = true;
    int transparency = 0;
    LayerBlend blend = LayerBlend::Normal;
    // shortcut: masks stay in canvas coordinates; add linked transforms with on-canvas effect handles.
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
    QUuid addLayer(EffectKind effect = EffectKind::StaticColor);
    QUuid importImage(const QString& fileName);
    QUuid addComponent(const QString& componentId);
    QImage render(double seconds = 0) const;
    static QImage render(const LightingState& state, double seconds = 0);
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
