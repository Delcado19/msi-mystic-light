#include "canvas/LayoutGeometry.h"
#include <QWheelEvent>
#include "effects/EffectClock.h"
#include "ui/LightingCanvasWidget.h"
#include "ui/DeviceArtwork.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QImage>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QDebug>
#include <QShowEvent>
#include <QHideEvent>
#include <QSvgRenderer>
#include <algorithm>
#include <vector>
#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include "effects/IPositionableEffect.h"

namespace {
// The preview is deliberately cheaper than hardware rendering and stops
// when hidden. UI animation settings never change hardware effect timing.
constexpr int kRepaintIntervalMs = 1000 / 60;

}

LightingCanvasWidget::LightingCanvasWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
    setAccessibleName(QStringLiteral("Spatial lighting preview"));
    setToolTip(QStringLiteral("Click a component to edit it individually. Alt+drag moves the chain. Select, then drag the corner to resize proportionally or rotation arrow to rotate. Ctrl+wheel scales the component."));
    setCursor(Qt::OpenHandCursor);
#ifdef Q_OS_WIN
    BOOL animations = TRUE;
    if (SystemParametersInfoW(SPI_GETCLIENTAREAANIMATION, 0, &animations, 0)) reducedMotion_ = !animations;
#endif
    previewPaused_ = reducedMotion_;
    repaintTimer_.setTimerType(Qt::PreciseTimer);
    (void)effectTimeMs();
    connect(&repaintTimer_, &QTimer::timeout, this, [this] { update(); });
    if (!previewPaused_) repaintTimer_.start(kRepaintIntervalMs);
}

void LightingCanvasWidget::setChannel(const std::string& channelName, CanvasPlacement placement, CanvasSource source, std::shared_ptr<IEffect> ownEffect)
{
    channels_[channelName] = ChannelBox{placement, source, std::move(ownEffect)};
}

void LightingCanvasWidget::removeChannel(const std::string& channelName)
{
    channels_.erase(channelName);
    // The box being dragged just disappeared out from under the cursor;
    // drop the stale drag state rather than keep moving a box that no
    // longer exists in channels_.
    if (draggedChannel_ == channelName) {
        dragKind_ = DragTarget::None;
    }
}

void LightingCanvasWidget::setGlobalEffect(std::shared_ptr<IEffect> effect)
{
    globalEffect_ = std::move(effect);
}

IPositionableEffect* LightingCanvasWidget::globalEffectPositionable() const
{
    return globalEffect_ ? dynamic_cast<IPositionableEffect*>(globalEffect_.get()) : nullptr;
}

int LightingCanvasWidget::sceneWidth() const { return fitToView_ ? 1100 : width(); }
int LightingCanvasWidget::sceneHeight() const { return fitToView_ ? 700 : height(); }

QTransform LightingCanvasWidget::viewTransform() const
{
    if (!fitToView_) return {};
    const qreal scale = qMin(width() / 1100.0, height() / 700.0);
    QTransform transform;
    transform.translate((width() - 1100 * scale) / 2, (height() - 700 * scale) / 2);
    transform.scale(scale, scale);
    return transform;
}

QPoint LightingCanvasWidget::sceneToWidget(QPoint point) const { return viewTransform().map(point); }
QPoint LightingCanvasWidget::widgetToScene(QPoint point) const { return viewTransform().inverted().map(point); }

void LightingCanvasWidget::setSelectedChannel(const std::string& name)
{
    if (selectedChannel_ == name) return;
    selectedChannel_ = name;
    // Selection is immediate; pointer tracking and keyboard focus never wait for a fade.
    update();
}

void LightingCanvasWidget::setPreviewPaused(bool paused)
{
    if (paused == previewPaused_) return;
    previewTimeMs_ = effectTimeMs();
    previewPaused_ = paused;
    if (paused) repaintTimer_.stop();
    else if (isVisible()) repaintTimer_.start(kRepaintIntervalMs);
    update();
}

void LightingCanvasWidget::showEvent(QShowEvent* event)
{
    QOpenGLWidget::showEvent(event);
    if (!previewPaused_) repaintTimer_.start(kRepaintIntervalMs);
}

void LightingCanvasWidget::hideEvent(QHideEvent* event)
{
    repaintTimer_.stop();
    QOpenGLWidget::hideEvent(event);
}

void LightingCanvasWidget::initializeGL()
{
    qInfo() << "Canvas OpenGL renderer:" << reinterpret_cast<const char*>(context()->functions()->glGetString(GL_RENDERER));
}

void LightingCanvasWidget::paintGL()
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.fillRect(rect(), QColor("#10161e"));
    painter.setTransform(viewTransform());
    const int sw = sceneWidth(), sh = sceneHeight();
    const int64_t elapsedMs = previewPaused_ ? previewTimeMs_ : effectTimeMs();
    if (globalEffect_) {
        // A bounded color field is smoothly interpolated by the GL paint engine.
        // Hardware LEDs still sample exact scene coordinates, never this texture.
        // Sampling cost is independent of window size and high-DPI scaling.
        const int pw=qMin(sw,220),ph=qMin(sh,140);
        QImage preview(pw,ph,QImage::Format_RGB32);
        for (int y = 0; y < preview.height(); ++y) {
            auto* line = reinterpret_cast<QRgb*>(preview.scanLine(y));
            for (int x = 0; x < preview.width(); ++x) {
                const auto c = globalEffect_->colorAt(x * sw / pw, y * sh / ph, elapsedMs);
                line[x] = qRgb(c.r, c.g, c.b);
            }
        }
        painter.drawImage(QRect(0, 0, sw, sh), preview);
        painter.setOpacity(1.0);
    }
    componentHits_.clear();
    for (const auto& [name, box] : channels_) {
        if(hidden_.count(name))continue;
        const auto& placement=box.placement;const QRectF r(placement.x,placement.y,placement.width,placement.height);
        const bool selected=name==selectedChannel_;
        QTransform rotation;rotation.translate(r.center().x(),r.center().y());rotation.rotate(placement.rotation);rotation.translate(-r.center().x(),-r.center().y());
        const auto geometry=geometries_.find(name);
        painter.save();painter.setWorldTransform(rotation,true);
        painter.setPen(QPen(QColor(238,245,255,210),1.4));painter.setBrush(QColor(10,16,24,70));
        if(geometry==geometries_.end() || geometry->second.empty()) {
            painter.drawRoundedRect(r,6,6);
        } else {
            // Product artwork and address-bound light markers share the layout transform.
            // The separate LED dialog provides readable key/zone editing.
            const auto bounds=layoutBounds(geometry->second);
            for(const auto& component:geometry->second) {
                int left=0,top=0,right=0,bottom=0;bool first=true;
                for(int i=0;i<component.ledCount;++i){
                    if(component.ledNames[i].find("Wire slot")==0)continue;
                    const auto [x,y]=component.ledCoordinates[i];
                    if(first){left=right=x;top=bottom=y;first=false;}
                    left=qMin(left,x);right=qMax(right,x);top=qMin(top,y);bottom=qMax(bottom,y);
                }
                if(first)continue;
                const double x=r.x()+(left-bounds.left)*(r.width()-1)/qMax(1,bounds.right-bounds.left);
                const double y=component.type=="KeyboardZones"?r.y():r.y()+(top-bounds.top)*(r.height()-1)/qMax(1,bounds.bottom-bounds.top);
                const double w=qMax(12.,(right-left)*(r.width()-1)/qMax(1,bounds.right-bounds.left));
                const double h=component.type=="KeyboardZones"?r.height():qMax(12.,(bottom-top)*(r.height()-1)/qMax(1,bounds.bottom-bounds.top));
                // A one-zone GPU or I/O cover has no spatial LED extent, but its
                // illustration still needs the full movable device footprint.
                QRectF shape=component.ledCount==1?r:QRectF(x,y,w,h);
                if(deviceArtworkKind(component)=="ram")shape.setHeight(r.height());
                if(deviceArtworkKind(component)=="fan"){const double side=qMax(w,h);shape=QRectF(shape.center()-QPointF(side/2,side/2),QSizeF(side,side));}
                const int componentIndex=int(&component-geometry->second.data());
                componentHits_.push_back({name,componentIndex,shape});
                if(selected && selectedComponent_==componentIndex){
                    painter.setPen(QPen(QColor("#a8f3de"),1.5));painter.setBrush(Qt::NoBrush);painter.drawRect(shape.adjusted(-4,-4,4,4));
                    painter.setBrush(QColor("#71e2c5"));painter.drawRect(QRectF(shape.bottomRight()-QPointF(5,5),QSizeF(10,10)));
                    static QSvgRenderer icon(QStringLiteral(":/studio/rotate.svg"));icon.render(&painter,QRectF(shape.center().x()-10,shape.top()-30,20,20));
                }
                if(component.type=="Keyboard") {
                    // keyboard.svg's own 600x175 aspect ratio doesn't match this
                    // layout's actual shape, and QSvgRenderer::render(painter,rect)
                    // fits it into rect preserving that aspect ratio -- letterboxed,
                    // so the drawn border undershoots the key area on one axis: wide
                    // gutters on the slack axis, keys poking past the border on the
                    // constrained one. Draw the deck directly instead, sized to the
                    // actual key bounds exactly, no aspect lock.
                    painter.setPen(QPen(QColor("#8a9bb0"),1.4));painter.setBrush(QColor("#141e2b"));
                    painter.drawRoundedRect(shape.adjusted(-w*.03,-h*.12,w*.03,h*.12),6,6);
                } else {
                    drawDeviceArtwork(painter,shape.adjusted(-w*.03,-h*.09,w*.03,h*.09),component);
                }
                const auto sample=box.source==CanvasSource::GlobalFollow?globalEffect_:box.ownEffect;
                for(int led=0;led<component.ledCount;++led) {
                    if(component.ledNames[led].find("Wire slot")==0)continue;
                    const auto [lx,ly]=component.ledCoordinates[led];
                    const QPointF dot(r.x()+(lx-bounds.left)*(r.width()-1)/qMax(1,bounds.right-bounds.left),r.y()+(ly-bounds.top)*(r.height()-1)/qMax(1,bounds.bottom-bounds.top));
                    const auto coords=layoutSamplePoint(component.ledCoordinates[led],bounds,placement,box.source==CanvasSource::GlobalFollow);
                    int offset=0,total=0;for(const auto& c:geometry->second){if(&c==&component)offset=total;total+=c.ledCount;}
                    const int address=offset+component.ledMapping[led];
                    auto color=sample?(box.source==CanvasSource::GlobalFollow?sample->colorAt(coords.first,coords.second,elapsedMs):sample->colorForLed(coords.first,coords.second,elapsedMs,offset+component.effectAddress(led),total,component.effectAddress(led),component.ledCount)):MsiColor{};
                    const auto fixed=overrides_.find(name);if(fixed!=overrides_.end()){const auto override=fixed->second.find(address);if(override!=fixed->second.end())color=override->second;}
                    const QColor rgb(color.r,color.g,color.b);
                    if(component.type=="Keyboard") {
                        const double unit=w/qMax(1,right-left),keyH=qMax(2.,h/qMax(1,bottom-top)*.76);
                        double keyW=unit*.78;if(led==0x25)keyW=unit*6.2;else if(led==0x52)keyW=unit*2.;
                        const QRectF key(dot.x()-keyW/2,dot.y()-keyH/2,keyW,keyH);
                        painter.setPen(QPen(QColor(100,117,140),.6));painter.setBrush(QColor("#182330"));painter.drawRoundedRect(key,2,2);
                        painter.fillRect(QRectF(key.left()+1,key.bottom()-qMax(1.,keyH*.16),qMax(1.,keyW-2),qMax(1.,keyH*.12)),rgb);
                        if(keyW>12 && keyH>12){auto f=font();f.setPixelSize(qBound(7,int(keyH*.48),11));painter.setFont(f);painter.setPen(QColor("#e4ecf7"));painter.drawText(key,Qt::AlignCenter,QString::fromStdString(component.ledNames[led]).left(3));}
                    } else if(component.type=="KeyboardZones"){
                        painter.fillRect(QRectF(x+w*led/component.ledCount,y+h*.92,w/component.ledCount-1,h*.055),rgb);
                    } else {
                        const double radius=qBound(2.,qMin(w,h)*.026,7.);
                        painter.setPen(QPen(QColor(225,235,250,160),.8));painter.setBrush(rgb);painter.drawEllipse(dot,radius,radius);
                        if(component.physicalLedsPerAddress>1)painter.drawEllipse(dot+QPointF(radius*2.4,0),radius,radius);
                    }
                }
            }
        }
        if(selected && selectedComponent_<0){painter.setPen(QPen(QColor("#a8f3de"),2));painter.setBrush(Qt::NoBrush);painter.drawRoundedRect(r.adjusted(-4,-4,4,4),5,5);painter.setBrush(QColor("#71e2c5"));painter.drawRect(QRectF(r.right()-5,r.bottom()-5,10,10));painter.save();
            static QSvgRenderer rotateIcon(QStringLiteral(":/studio/rotate.svg"));
            rotateIcon.render(&painter,QRectF(r.center().x()-10,r.top()-30,20,20));painter.restore();}
        painter.restore();
        painter.save();painter.resetTransform();
        // Persistent labels on nearby hardware overlap; selection supplies context.
        if(!selected){painter.restore();continue;}
        const QPoint anchor=sceneToWidget(rotation.map(QPointF(r.center().x(),r.bottom())).toPoint());
        const QRect textRect(anchor.x()-60,anchor.y()+6,120,23);painter.setPen(Qt::NoPen);painter.setBrush(QColor(16,22,30,230));painter.drawRoundedRect(textRect,5,5);
        painter.setPen(selected?QColor("#a8f3de"):QColor("#d5dfeb"));QFont f=font();f.setPointSize(9);painter.setFont(f);
        const QString title=QString::fromStdString(name);painter.drawText(textRect,Qt::AlignCenter,painter.fontMetrics().elidedText(title,Qt::ElideRight,112));painter.restore();
    }
    if (auto* positionable = globalEffectPositionable()) {
        const auto [rx, ry] = positionable->referencePoint();
        painter.setPen(QPen(QColor("#71e2c5"), 2));
        painter.setBrush(QColor("#111a23"));
        painter.drawEllipse(QPoint(rx, ry), kHandleHitRadius / 2, kHandleHitRadius / 2);
    }
}

void LightingCanvasWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) return;

    const QPoint pos = widgetToScene(event->position().toPoint());

    // Hit-test in reverse paint order (paintEvent draws the handle last,
    // on top of every box; see below): the handle is checked first, then
    // boxes from topmost-drawn to bottommost-drawn. Checking boxes first
    // in forward map order would let a box hidden under the handle (or
    // under another box) steal clicks meant for whatever is actually
    // visible on top -- e.g. a motherboard zone box sitting under a
    // radial effect's draggable center.
    if (auto* positionable = globalEffectPositionable()) {
        const auto [rx, ry] = positionable->referencePoint();
        if ((pos - QPoint(rx, ry)).manhattanLength() <= kHandleHitRadius) {
            dragKind_ = DragTarget::GlobalReferencePoint;
            return;
        }
    }

    for(auto hit=componentHits_.rbegin();hit!=componentHits_.rend();++hit) {
        if(!channels_.count(hit->channel) || hidden_.count(hit->channel))continue;
        // Standalone devices use placement handles; flat RAM LED rows cannot define artwork height.
        if(geometries_.at(hit->channel).size()==1)continue;
        const auto& placement=channels_.at(hit->channel).placement;const QRectF frame(placement.x,placement.y,placement.width,placement.height);
        QTransform rotation;rotation.translate(frame.center().x(),frame.center().y());rotation.rotate(placement.rotation);rotation.translate(-frame.center().x(),-frame.center().y());
        const auto local=rotation.inverted().map(QPointF(pos));DragTarget kind=DragTarget::None;
        const bool active=hit->channel==selectedChannel_ && hit->index==selectedComponent_;
        if(active && QLineF(local,hit->rect.bottomRight()).length()<14)kind=DragTarget::ComponentResize;
        else if(active && QLineF(local,QPointF(hit->rect.center().x(),hit->rect.top()-20)).length()<14)kind=DragTarget::ComponentRotate;
        // A standalone device moves through placement; normalizing its LED bounds
        // would otherwise cancel the visible translation for local effects.
        else if(!(event->modifiers()&Qt::AltModifier) && geometries_.at(hit->channel).size()>1 && hit->rect.contains(local))kind=DragTarget::ComponentMove;
        if(kind==DragTarget::None)continue;
        dragKind_=kind;draggedChannel_=hit->channel;selectedComponent_=hit->index;dragComponents_=geometries_.at(hit->channel);dragStartPlacement_=placement;
        for(auto& c:dragComponents_)if(c.layoutDefaults.empty())c.layoutDefaults=c.ledCoordinates;
        const auto bounds=layoutBounds(dragComponents_);
        const auto toLocal=[&](QPointF p){return QPointF(bounds.left+(p.x()-placement.x)*qMax(1,bounds.right-bounds.left)/(placement.width-1),bounds.top+(p.y()-placement.y)*qMax(1,bounds.bottom-bounds.top)/(placement.height-1));};
        componentCenter_=toLocal(hit->rect.center());componentGrab_=toLocal(local);setSelectedChannel(hit->channel);emit channelSelected(hit->channel);update();return;
    }
    selectedComponent_=-1;
    for (auto it=channels_.rbegin();it!=channels_.rend();++it) {
        if(hidden_.count(it->first))continue;
        const auto& p=it->second.placement;const QRectF r(p.x,p.y,p.width,p.height);
        QTransform rotation;rotation.translate(r.center().x(),r.center().y());rotation.rotate(p.rotation);rotation.translate(-r.center().x(),-r.center().y());
        const auto local=rotation.inverted().map(QPointF(pos));
        DragTarget kind=DragTarget::None;
        // A little larger than the drawn handle (10px radius): Apple's hit-target
        // guidance calls for forgiveness around a small drag target, not a hitbox
        // that matches the artwork pixel-for-pixel.
        if(it->first==selectedChannel_ && QLineF(local,r.bottomRight()).length()<14)kind=DragTarget::Resize;
        else if(it->first==selectedChannel_ && QLineF(local,QPointF(r.center().x(),r.top()-20)).length()<14)kind=DragTarget::Rotate;
        else if(r.contains(local))kind=DragTarget::Channel;
        if(kind!=DragTarget::None){
            dragKind_=kind;draggedChannel_=it->first;dragStartPlacement_=p;setSelectedChannel(it->first);emit channelSelected(it->first);dragOffset_=pos-QPoint(qRound(p.x),qRound(p.y));setCursor(Qt::ClosedHandCursor);
            if(kind==DragTarget::Rotate){const QPointF center(p.x+p.width/2,p.y+p.height/2),grab=QPointF(pos)-center;dragStartAngle_=std::atan2(grab.y(),grab.x())*180./3.141592653589793;}
            return;
        }
    }

    dragKind_ = DragTarget::None;
}

void LightingCanvasWidget::mouseMoveEvent(QMouseEvent* event)
{
    const QPoint pos = widgetToScene(event->position().toPoint());

    if(dragKind_==DragTarget::ComponentMove || dragKind_==DragTarget::ComponentResize || dragKind_==DragTarget::ComponentRotate) {
        if(!channels_.count(draggedChannel_) || selectedComponent_<0 || selectedComponent_>=int(dragComponents_.size())){dragKind_=DragTarget::None;return;}
        const auto& placement=dragStartPlacement_;const QRectF frame(placement.x,placement.y,placement.width,placement.height);
        QTransform rotation;rotation.translate(frame.center().x(),frame.center().y());rotation.rotate(placement.rotation);rotation.translate(-frame.center().x(),-frame.center().y());
        const auto local=rotation.inverted().map(QPointF(pos));const auto bounds=layoutBounds(dragComponents_);
        const QPointF point(bounds.left+(local.x()-placement.x)*qMax(1,bounds.right-bounds.left)/(placement.width-1),bounds.top+(local.y()-placement.y)*qMax(1,bounds.bottom-bounds.top)/(placement.height-1));
        auto changed=dragComponents_;const auto delta=point-componentCenter_,initial=componentGrab_-componentCenter_;
        const double factor=qBound(.1,std::hypot(delta.x(),delta.y())/qMax(1.,std::hypot(initial.x(),initial.y())),10.);
        const double angle=std::atan2(delta.y(),delta.x())-std::atan2(initial.y(),initial.x());
        for(auto& coord:changed[selectedComponent_].ledCoordinates){QPointF p(coord.first,coord.second);if(dragKind_==DragTarget::ComponentMove)p+=point-componentGrab_;else {p-=componentCenter_;if(dragKind_==DragTarget::ComponentResize)p*=factor;else p=QPointF(p.x()*std::cos(angle)-p.y()*std::sin(angle),p.x()*std::sin(angle)+p.y()*std::cos(angle));p+=componentCenter_;}coord={qBound(-100000,qRound(p.x()),100000),qBound(-100000,qRound(p.y()),100000)};}
        geometries_[draggedChannel_]=changed;emit componentGeometryChanged(draggedChannel_,changed);update();return;
    }
    if (dragKind_ == DragTarget::Channel) {
        auto it = channels_.find(draggedChannel_);
        if (it == channels_.end()) { dragKind_ = DragTarget::None; return; }
        const QPoint newTopLeft = pos - dragOffset_;
        // Qt keeps delivering move/release events past the widget's own
        // rect() while a button is held (including negative coordinates),
        // so an unclamped drag released past an edge would leave the box
        // positioned off-canvas with no way to click it again -- and that
        // off-canvas value is exactly what channelPlacementChanged persists.
        // Clamp so the box stays fully visible within the widget.
        const int maxX = qMax(0, sceneWidth() - qRound(it->second.placement.width));
        const int maxY = qMax(0, sceneHeight() - qRound(it->second.placement.height));
        const int clampedX = qBound(0, newTopLeft.x(), maxX);
        const int clampedY = qBound(0, newTopLeft.y(), maxY);
        it->second.placement.x=clampedX;it->second.placement.y=clampedY;
        emit channelPlacementChanged(draggedChannel_, it->second.placement);
        update();
    } else if(dragKind_==DragTarget::Resize || dragKind_==DragTarget::Rotate) {
        auto it=channels_.find(draggedChannel_);if(it==channels_.end())return;
        auto p=dragStartPlacement_;const QPointF center(p.x+p.width/2,p.y+p.height/2),delta=QPointF(pos)-center;
        if(dragKind_==DragTarget::Rotate) {
            // Relative to the angle at grab, not the pointer's absolute angle from
            // center -- otherwise any grab that isn't pixel-perfect on the handle
            // snaps the box to the pointer's angle instead of tracking the drag.
            const double currentAngle=std::atan2(delta.y(),delta.x())*180./3.141592653589793;
            p.rotation=std::remainder(dragStartPlacement_.rotation+(currentAngle-dragStartAngle_),360.);
        }
        else {
            const double angle=-p.rotation*3.141592653589793/180.;
            const double dx=delta.x()*std::cos(angle)-delta.y()*std::sin(angle),dy=delta.x()*std::sin(angle)+delta.y()*std::cos(angle);
            // Project onto the original corner direction so a one-axis drag
            // scales both dimensions without changing the hardware aspect ratio.
            const double factor=qBound(qMax(10./p.width,10./p.height),2*(dx*p.width+dy*p.height)/(p.width*p.width+p.height*p.height),qMin(2000./p.width,2000./p.height));
            p.width*=factor;p.height*=factor;p.x=center.x()-p.width/2;p.y=center.y()-p.height/2;
        }
        it->second.placement=p;emit channelPlacementChanged(draggedChannel_,p);update();
    } else if (dragKind_ == DragTarget::GlobalReferencePoint) {
        if (auto* positionable = globalEffectPositionable()) {
            // Same off-canvas-drag issue as the channel-box branch above,
            // but for the global effect's reference-point handle: clamp so
            // the handle (drawn with radius kHandleHitRadius/2) stays fully
            // visible and re-clickable afterward.
            const int handleRadius = kHandleHitRadius / 2;
            const int maxX = qMax(handleRadius, sceneWidth() - handleRadius);
            const int maxY = qMax(handleRadius, sceneHeight() - handleRadius);
            const int clampedX = qBound(handleRadius, pos.x(), maxX);
            const int clampedY = qBound(handleRadius, pos.y(), maxY);
            positionable->setReferencePoint(clampedX, clampedY);
            emit globalEffectReferencePointChanged(clampedX, clampedY);
            update();
        }
    }
}

void LightingCanvasWidget::mouseReleaseEvent(QMouseEvent*)
{
    if (dragKind_ == DragTarget::Channel || dragKind_==DragTarget::Resize || dragKind_==DragTarget::Rotate) {
        auto it = channels_.find(draggedChannel_);
        if (it != channels_.end()) {
            // One more emission on release, per the public contract (see
            // header): lets callers persist the final placement without
            // having to debounce every live drag update themselves.
            emit channelPlacementChanged(draggedChannel_, it->second.placement);
        }
    }
    dragKind_ = DragTarget::None;
    draggedChannel_.clear();
    setCursor(Qt::OpenHandCursor);
}

void LightingCanvasWidget::wheelEvent(QWheelEvent* event) {
    auto it=channels_.find(selectedChannel_);if(it==channels_.end() || !(event->modifiers()&Qt::ControlModifier)){event->ignore();return;}
    auto p=it->second.placement;const double factor=event->angleDelta().y()>0?1.1:1/1.1;
    const double boundedFactor=qBound(qMax(10./p.width,10./p.height),factor,qMin(2000./p.width,2000./p.height));
    const double width=p.width*boundedFactor,height=p.height*boundedFactor;
    p.x+=(p.width-width)/2;p.y+=(p.height-height)/2;p.width=width;p.height=height;it->second.placement=p;emit channelPlacementChanged(it->first,p);update();event->accept();
}
