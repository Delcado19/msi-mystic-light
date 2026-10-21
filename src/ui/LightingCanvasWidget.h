// Spatial preview in the same scene coordinates used by Canvas::tick.
// The inspector fits the 1100x700 workspace into the available viewport;
// pointer positions are inverse-transformed, so resizing never changes
// physical placement or hardware sampling. Standalone widgets keep 1:1
// coordinates unless setFitToView(true) is requested.
// Preview frames run at up to 60 Hz, stop while hidden/paused, and are independent
// of hardware rendering. The effect field is the spatial sampling surface;
// artwork carries addressed light markers; detailed color editing has its own dialog.
#pragma once

#include <QPoint>
#include <QTimer>
#include <QOpenGLWidget>
#include <QTransform>

#include <map>
#include <memory>
#include <string>

#include "canvas/Canvas.h" // CanvasPlacement, CanvasSource
#include "effects/IEffect.h"

class IPositionableEffect; // effects/IPositionableEffect.h -- only referenced by pointer here

class LightingCanvasWidget : public QOpenGLWidget {
    Q_OBJECT
public:
    explicit LightingCanvasWidget(QWidget* parent = nullptr);

    // Registers (or replaces) one channel's canvas box; `name` is shown as
    // the box's label. Placement/source come from the ChannelSettings the
    // caller already owns -- this widget does not read Settings directly.
    void setChannel(const std::string& channelName, CanvasPlacement placement, CanvasSource source, std::shared_ptr<IEffect> ownEffect);
    void removeChannel(const std::string& channelName);
    void setGlobalEffect(std::shared_ptr<IEffect> effect);
    void setChannelOverrides(const std::string& name, const std::map<int,MsiColor>& overrides) { overrides_[name]=overrides; update(); }
    void setChannelComponents(const std::string& name, const std::vector<Component>& components) { geometries_[name]=components; update(); }

    // View transforms never change persisted placement or effect sampling.
    void setFitToView(bool enabled) { fitToView_ = enabled; update(); }
    QPoint sceneToWidget(QPoint point) const;
    void setSelectedChannel(const std::string& name);
    void setPreviewPaused(bool paused);
    bool previewPaused() const { return previewPaused_; }


    // Legacy default footprint for callers without custom sizing. Actual
    // rendering/sampling uses CanvasPlacement width, height and rotation.
    static constexpr int kBoxWidth = kGlobalFollowBoxWidth;
    static constexpr int kBoxHeight = kGlobalFollowBoxHeight;
    static constexpr int kHandleHitRadius = 10;

signals:
    void channelSelected(const std::string& channelName);
    // Emitted live while dragging a channel's box, and once more on
    // release. The widget already applies the new placement to its own
    // channels_ entry before emitting -- this signal is a notification for
    // the caller to persist (e.g. into Settings), not a request the
    // caller must apply back into the widget itself.
    void channelPlacementChanged(const std::string& channelName, CanvasPlacement newPlacement);
    // Emitted while dragging a positionable global effect's reference-point
    // handle. Like channelPlacementChanged, the widget has already called
    // setReferencePoint() on the (shared, mutated-in-place) global effect
    // object before emitting; this is a notification, not a request.
    void globalEffectReferencePointChanged(int x, int y);

protected:
    void paintGL() override;
    void initializeGL() override;
    void showEvent(QShowEvent*) override;
    void hideEvent(QHideEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;

private:
    struct ChannelBox { CanvasPlacement placement; CanvasSource source; std::shared_ptr<IEffect> ownEffect; };

    // Returns globalEffect_ re-exposed as IPositionableEffect, or nullptr
    // if unset or the assigned effect doesn't implement that interface
    // (see IPositionableEffect.h -- only some effects, e.g.
    // RadialRainbowEffect, expose a draggable reference point).
    IPositionableEffect* globalEffectPositionable() const;

    QTransform viewTransform() const;
    QPoint widgetToScene(QPoint point) const;
    int sceneWidth() const;
    int sceneHeight() const;
    bool fitToView_ = false;
    bool previewPaused_ = false;
    bool reducedMotion_ = false;
    std::string selectedChannel_;
    int64_t previewTimeMs_ = 0;
    std::map<std::string, std::map<int,MsiColor>> overrides_;
    std::map<std::string, std::vector<Component>> geometries_;
    std::map<std::string, ChannelBox> channels_;
    std::shared_ptr<IEffect> globalEffect_;
    QTimer repaintTimer_;


    // Drag state: which box (by name) or "the global effect's reference
    // point" is currently grabbed, and the press-time offset, set in
    // mousePressEvent and read in mouseMoveEvent/mouseReleaseEvent.
    enum class DragTarget { None, Channel, Resize, Rotate, GlobalReferencePoint };
    CanvasPlacement dragStartPlacement_;
    DragTarget dragKind_ = DragTarget::None;
    std::string draggedChannel_;
    QPoint dragOffset_; // press position minus the dragged box's top-left, so the box doesn't jump to re-center on the cursor
};
