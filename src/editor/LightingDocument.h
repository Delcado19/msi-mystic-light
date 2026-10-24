#pragma once

#include "components/ComponentCatalog.h"
#include "EffectRenderer.h"
#include "ChannelAssignment.h"
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
    QVector<QColor> gradientColors; // Empty preserves the legacy two-color gradient. Otherwise 2..16 evenly spaced stops.
    bool gradientFill = false;
    QPointF gradientStart{0,.5}, gradientEnd{1,.5}; // Normalized shape-local fill endpoints; geometry stays editable.
    QVector<double> gradientPositions; // Empty means evenly spaced, preserving schemas 1..10.
    QPointF center{320, 200};
    QSizeF extent{240, 240};
    double rotation = 0;
    int repetitions = 4;
    QImage image;
    bool imageFill = false; // Fit preserves the whole image; fill crops to the effect rectangle.
    double spin = 0, pulse = 0, hueShift = 0; // Degrees/second, cycles/second, degrees/second.
    double driftX = 0, driftY = 0, zoom = 0, warp = 0, breathe = 0, trails = 0, cycleRate = 0;
    double particleCount = 80, particleSeed = 1, particleSpeed = 60, particleSize = 3;
    QVector<QColor> colorCycle{Qt::red, Qt::green, Qt::blue};
    QRectF crop{0, 0, 1, 1}; // Normalized, non-destructive source-image selection.
    bool maskLinked = false;
    bool allChannels = true;
    QVector<QUuid> channelTargets; // Empty with allChannels=false means no hardware target, not all.
    bool visible = true;
    int transparency = 0;
    LayerBlend blend = LayerBlend::Normal;
    // Unlinked masks stay in canvas coordinates; linked masks follow geometry/motion.
    QImage mask;
    bool maskEnabled = true;
};
struct ComponentPlacement {
    QUuid id = QUuid::createUuid();
    QString componentId;
    QPointF origin{80, 80};
    QSizeF scale{1, 1};
    double rotation = 0; // Degrees around the component origin, after scaling catalog coordinates.
};
struct LightingState {
    QVector<ChannelAssignment> assignments;
    ComponentCatalog components; // Document-owned geometry participates in undo and portable saves.
    QVector<ComponentPlacement> placements;
    QVector<EffectLayer> layers; // Bottom to top; panel presentation is reversed.
};

// Offline document only. Catalog geometry is never inferred as attached hardware.
class LightingDocument : public QObject {
    Q_OBJECT
public:
    explicit LightingDocument(ComponentCatalog catalog, QObject* parent = nullptr, QVector<DeviceProfile> profiles = {});
    static QSize canvasSize() { return {640, 400}; }
    const ComponentCatalog& catalog() const { return state_.components; }
    const LightingState& state() const { return state_; }
    const QVector<DeviceProfile>& profiles() const { return profiles_; }
    quint64 revision() const { return revision_; }
    QUndoStack* history() { return &history_; }
    void edit(const QString& label, const std::function<void(LightingState&)>& operation);
    QUuid addLayer(EffectKind effect = EffectKind::StaticColor);
    QUuid addPaintLayer();
    QUuid importImage(const QString& fileName);
    void defineComponent(const ComponentDefinition& component, bool replace = false);
    // Isolates geometry for one placement; physical LED count and wire order stay unchanged.
    void setComponentGeometry(const QUuid& placement, const QVector<QPointF>& positions);
    void importComponent(const QString& fileName);
    QUuid addComponent(const QString& componentId);
    QVector<QPointF> componentPoints(const ComponentPlacement& placement) const;
    // Per-instance wire order, opaque RGB over black; optional placement previews never mutate undo history.
    QMap<QUuid, QVector<QColor>> sampleComponents(const QImage& frame, const QVector<ComponentPlacement>* placements = nullptr) const;
    void assignChannel(const ChannelAssignment& assignment);
    QMap<QUuid, QVector<QColor>> sampleChannels(const QImage& frame, double seconds = 0, const QVector<ComponentPlacement>* placements = nullptr) const;
    double channelTime(const QUuid& assignment, double seconds) const;
    QImage renderForChannel(const QUuid& assignment, double seconds) const;
    QImage render(double seconds = 0) const;
    static QImage render(const LightingState& state, double seconds = 0, const QUuid& channel = {});
    static QImage whiteMask();
    void save(const QString& fileName, bool markClean = true);
    void load(const QString& fileName);
    static void paintMask(QImage& mask, QPointF from, QPointF to,
        double radius, double hardness, double opacity, int gray);
    static void gradientMask(QImage& mask, QPointF from, QPointF to);
signals:
    void changed();
private:
    QVector<QColor> sampleComponent(const ComponentPlacement& placement, const QImage& premultiplied) const;
    const QVector<DeviceProfile> profiles_;
    const ComponentCatalog initialCatalog_;
    LightingState state_;
    QUndoStack history_;
    quint64 revision_ = 0; // Session-only monotonic invalidation for confirmed output snapshots.
};
