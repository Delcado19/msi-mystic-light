#include "LightingCanvas.h"
#include "SnapGrid.h"
#include "HardwareIdentity.h"
#include "HardwareArtwork.h"
#include <QTimer>
#include <QApplication>
#include "StudioWidgets.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QPushButton>
#include <QResizeEvent>
#include <QLineF>
#include <QPolygonF>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <QSet>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QStyle>
#include <QMenu>

namespace {
bool centeredGradient(EffectKind kind) { return kind==EffectKind::RadialGradient || kind==EffectKind::ConicalGradient; }
QPair<QPointF,QPointF> gradientEndpoints(const EffectLayer& layer, double seconds) {
    const auto transform=effectTransform(layer,seconds);
    if(layer.gradientFill) return {transform.map(layer.gradientStart-QPointF(.5,.5)),transform.map(layer.gradientEnd-QPointF(.5,.5))};
    return {transform.map(QPointF(centeredGradient(layer.effect) ? 0 : -.5,0)),transform.map(QPointF(.5,0))};
}
}

LightingCanvas::LightingCanvas(LightingDocument& document, Context context, QWidget* parent)
    : QWidget(parent), document_(document), context_(context)
{
    showComponents_=context==Context::Layout;
    setMinimumSize(360, 280);
    autoScroll_=new QTimer(this); autoScroll_->setInterval(16); autoScroll_->setTimerType(Qt::PreciseTimer);
    connect(autoScroll_,&QTimer::timeout,this,&LightingCanvas::scrollAtEdge);
    fullscreen_=new QPushButton(this);fullscreen_->setIcon(studioToolIcon("fullscreen"));fullscreen_->setObjectName("canvasFullscreenButton");
    fullscreen_->setFixedSize(36,32);fullscreen_->setToolTip("Toggle canvas fullscreen (Escape to return)");
    fullscreen_->setAccessibleName("Canvas fullscreen");
    fullscreen_->setVisible(context==Context::Layout);
    connect(fullscreen_,&QPushButton::clicked,this,&LightingCanvas::fullscreenRequested);
    horizontalScroll_=new QScrollBar(Qt::Horizontal,this);horizontalScroll_->setObjectName("canvasHorizontalScroll");
    verticalScroll_=new QScrollBar(Qt::Vertical,this);verticalScroll_->setObjectName("canvasVerticalScroll");
    horizontalScroll_->setAccessibleName("Canvas horizontal scroll");verticalScroll_->setAccessibleName("Canvas vertical scroll");
    horizontalScroll_->hide();verticalScroll_->hide();
    for(auto* bar : {horizontalScroll_,verticalScroll_}) connect(bar,&QScrollBar::sliderPressed,this,&LightingCanvas::cancelGesture);
    connect(horizontalScroll_,&QScrollBar::valueChanged,this,[this](int value) {
        const auto bounds=navigationBounds();autoFit_=false;
        pan_.rx()+=contentRect().left()-value*scrollUnits_.width()-viewportPoint(bounds.topLeft()).x();update();
    });
    connect(verticalScroll_,&QScrollBar::valueChanged,this,[this](int value) {
        const auto bounds=navigationBounds();autoFit_=false;
        pan_.ry()+=contentRect().top()-value*scrollUnits_.height()-viewportPoint(bounds.topLeft()).y();update();
    });
    emptyAction_=new QPushButton(context==Context::Layout ? "Add a component" : "Add an effect layer",this);
    emptyAction_->setVisible(context==Context::Effects ? document_.state().layers.isEmpty() : document_.state().placements.isEmpty());
    emptyAction_->setObjectName("canvasAdd"); emptyAction_->setProperty("primary",true); emptyAction_->setFixedSize(190,38);
    connect(emptyAction_,&QPushButton::clicked,this,&LightingCanvas::addRequested);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setAccessibleName(context == Context::Layout ? "Component layout canvas" : "Effect and layer mask canvas");
    connect(&document_, &LightingDocument::changed, this, [this] {
        cancelGesture();
        if(previewOnly_ || (context_==Context::Effects && autoFit_)) fitToView();
        rendered_ = previewChannel_.isNull() ? document_.render(seconds_) : LightingDocument::render(document_.state(),seconds_,previewChannel_);
        emptyAction_->setVisible(!previewOnly_ && (context_==Context::Effects ? document_.state().layers.isEmpty() : document_.state().placements.isEmpty()));
        update();
    });
    rendered_ = previewChannel_.isNull() ? document_.render(seconds_) : LightingDocument::render(document_.state(),seconds_,previewChannel_);
}

void LightingCanvas::setEditorView(bool effects,bool components)
{
    cancelGesture();showEffects_=effects;showComponents_=components;update();
}
void LightingCanvas::copyCamera(const LightingCanvas& source)
{
    if(&source==this)return;
    cancelGesture();autoFit_=false;zoom_=source.zoom_;
    const auto bounds=document_.layoutBounds();
    const auto anchor=context_==Context::Layout ? bounds.topLeft() : QPointF();
    const auto sourceAnchor=source.context_==Context::Layout ? bounds.topLeft() : QPointF();
    pan_+=source.viewportPoint(sourceAnchor)-viewportPoint(anchor);update();
}

void LightingCanvas::setPreviewOnly() {previewOnly_=true;fullscreen_->hide();setMinimumHeight(160);setFocusPolicy(Qt::NoFocus);}

void LightingCanvas::fitToView()
{
    cancelGesture(); autoFit_=true; pan_={};
    if(previewOnly_) {
        previewBounds_=document_.layoutBounds();
        zoom_=std::clamp(std::min((width()-60)/previewBounds_.width(),(height()-64)/previewBounds_.height()),.001,2.0);
    } else {
        const auto bounds=context_==Context::Layout ? navigationBounds() : QRectF(QPointF(),document_.layoutBounds().size());
        zoom_=std::clamp(std::min((width()-80)/bounds.width(),(height()-100)/bounds.height()),0.001,4.0);
        pan_=context_==Context::Layout ? (QPointF(320,200)-bounds.center())*zoom_ : QPointF();
    }
    update();
}
void LightingCanvas::resizeEvent(QResizeEvent*)
{
    fullscreen_->move(width()-48,10);fullscreen_->setVisible(!previewOnly_ && context_==Context::Layout);
    if(autoFit_) fitToView();
    if(previewOnly_) emptyAction_->hide();
    emptyAction_->move((width()-emptyAction_->width())/2,height()/2+32);
}

void LightingCanvas::setPreviewFrame(const QImage& frame, double seconds, const QUuid& channel)
{
    seconds_ = seconds; previewChannel_=channel; rendered_ = frame; update();
}

QSizeF LightingCanvas::displayScale() const
{
    // Keep stored effects/masks in their canonical raster, but display them over the same world as hardware sampling.
    const auto bounds=document_.layoutBounds();
    return context_==Context::Effects ? QSizeF(bounds.width()/640.0,bounds.height()/400.0) : QSizeF(1,1);
}
QPointF LightingCanvas::offset() const
{
    if(previewOnly_) return QPointF(width()/2.0,height()/2.0+18)-previewBounds_.center()*zoom_;
    const auto scale=displayScale();
    return QPointF((width()-640*scale.width()*zoom_)/2,(height()-400*scale.height()*zoom_)/2)+pan_;
}
QPointF LightingCanvas::canvasPoint(QPointF viewport) const { const auto p=(viewport-offset())/zoom_;const auto scale=displayScale();return {p.x()/scale.width(),p.y()/scale.height()}; }
QPointF LightingCanvas::viewportPoint(QPointF canvas) const { const auto scale=displayScale();return offset()+QPointF(canvas.x()*scale.width(),canvas.y()*scale.height())*zoom_; }
void LightingCanvas::select(const QUuid& id)
{
    if(selected_==id)return;
    cancelGesture();selectedIds_.clear();if(!id.isNull())selectedIds_.insert(id);
    selected_ = id;
    emit selectionChanged();
    update();
}
void LightingCanvas::setMaskTarget(bool enabled) { if(maskTarget_==enabled)return;cancelGesture(); maskTarget_ = enabled; setCursor(enabled ? Qt::CrossCursor : Qt::ArrowCursor); update(); }
void LightingCanvas::setBrush(double radius, double hardness, double opacity, int gray)
{
    radius_ = radius; hardness_ = hardness; opacity_ = opacity; gray_ = gray; update();
}
void LightingCanvas::cancelGesture()
{
    const bool ending=context_==Context::Effects && dragging_;
    pendingDrawing_=false;createdObject_=false;marquee_=false;deferredHit_=false;guides_.clear();
    autoScroll_->stop();
    const bool restore=context_==Context::Layout && dragging_;
    drawing_=false; drawingBase_={}; fillReference_={};
    dragging_ = false;
    panning_ = false; navigating_=false;
    workingMask_ = {}; originalEffect_.reset(); workingEffect_.reset(); handle_ = 0; layoutScale_=1; layoutRotation_=0;
    if(restore) emit layoutPreviewChanged();
    if(ending) emit gestureEnded();
    update();
}

QVector<QPointF> LightingCanvas::componentPoints(const ComponentPlacement& placement) const
{
    return document_.componentPoints(placement);
}

void LightingCanvas::paintEvent(QPaintEvent*)
{
    syncScrollbars();
    QPainter painter(this);
    painter.fillRect(rect(), QColor("#17191f"));
    painter.setPen(QColor("#a6afc2"));
    const auto placementsForBounds=previewPlacements();
    const auto world=context_==Context::Layout ? document_.layoutBounds(&placementsForBounds) : QRectF(0,0,640,400);
    const auto displayedWorld=context_==Context::Layout ? world : document_.layoutBounds();
    painter.drawText(QRect(20,12,width()-70,24),Qt::AlignLeft|Qt::AlignVCenter,
        QString("%1    /    "+QString::number(std::ceil(displayedWorld.width()))+" x "+QString::number(std::ceil(displayedWorld.height()))+"    /    %2%").arg(previewOnly_ ? QString("LIVE HARDWARE") : context_==Context::Layout ? "LAYOUT" : "EFFECT CANVAS").arg(qRound(zoom_*100)));
    const bool empty=context_==Context::Effects ? document_.state().layers.isEmpty() : document_.state().placements.isEmpty();
    if(empty && previewOnly_) {
        painter.drawText(rect().adjusted(20,42,-20,-12),Qt::AlignCenter,"Attach hardware to see its live lighting here."); return;
    }
    if(empty) {
        painter.setPen(QColor("#e1e7f5")); auto font=painter.font(); font.setPointSize(18); painter.setFont(font);
        painter.drawText(QRect(20,height()/2-62,width()-40,38),Qt::AlignCenter,context_==Context::Layout ? "Place your lighting" : "Create your first effect");
        font.setPointSize(10); painter.setFont(font); painter.setPen(QColor("#acb6cb"));
        painter.drawText(QRect(20,height()/2-18,width()-40,32),Qt::AlignCenter,context_==Context::Layout ? "Arrange components here. Create their lighting in Effects." : "Choose a visual effect, then shape it directly on the canvas.");
        return;
    }
    painter.setClipRect(previewOnly_ ? QRectF(rect().adjusted(12,44,-12,-12)) : contentRect());
    painter.translate(offset());
    const auto scale=displayScale();
    painter.scale(zoom_*scale.width(),zoom_*scale.height());
    painter.setPen(QPen(QColor("#626d85"),1/zoom_)); painter.setBrush(Qt::NoBrush);
    if(!previewOnly_) {
    painter.drawRect(world.adjusted(-1,-1,1,1));
    painter.setClipRect(world,Qt::IntersectClip);
    // Bound checkerboard work to the visible world even when distant hardware expands the layout.
    const auto visible=world.intersected(QRectF(canvasPoint(QPointF()),canvasPoint(QPointF(width(),height()))));
    const double tile=16/std::min(1.0,zoom_);
    for(double y=std::floor(visible.top()/tile)*tile;y<visible.bottom();y+=tile)
        for(double x=std::floor(visible.left()/tile)*tile;x<visible.right();x+=tile)
            painter.fillRect(QRectF(x,y,tile,tile),std::fmod(std::floor(x/tile)+std::floor(y/tile),2.0)!=0 ? QColor("#484b50") : QColor("#35383d"));
    }
    auto frame = rendered_;
    if (context_ == Context::Effects) {
        if (workingEffect_) {
            auto state=document_.state();
            if(createdObject_)state.layers.append(*workingEffect_);
            else for (auto& layer : state.layers) if (selections().contains(layer.id)) layer=transformedSelection(layer);
            frame=LightingDocument::render(state,seconds_,previewChannel_);
        }
        if (maskTarget_) {
            auto state=document_.state();
            for (auto& layer : state.layers) if (layer.id==selected_) {
                if (!workingMask_.isNull()) layer.mask=workingMask_;
            }
            frame=LightingDocument::render(state,seconds_,previewChannel_); // Mask inspection must not freeze the live composition.
        }
        if (!workingMask_.isNull() && !maskTarget_) {
            auto state = document_.state();
            for (auto& layer : state.layers) if (layer.id == selected_) layer.mask = workingMask_;
            frame = LightingDocument::render(state, seconds_,previewChannel_);
        }
        if (showMask_ && maskTarget_) {
            for (const auto& layer : document_.state().layers)
                if (layer.id == selected_) {
                    auto preview=layer; if (!workingMask_.isNull()) preview.mask=workingMask_;
                    frame=preview.mask;
                }
        }
        if(showEffects_) painter.drawImage(0, 0, frame);
        if(showComponents_) {
            painter.save();const auto bounds=document_.layoutBounds();
            painter.scale(640/bounds.width(),400/bounds.height());painter.translate(-bounds.topLeft());
            paintComponents(painter,frame,bounds);painter.restore();
        }
        SnapGrid::load().paint(painter,QRectF(0,0,640,400).intersected(QRectF(canvasPoint({0,0}),canvasPoint({double(width()),double(height())}))),zoom_);
        if(maskTarget_ && tool_==MaskTool::Brush && underMouse()) {
            const auto cursor=canvasPoint(mapFromGlobal(QCursor::pos()));
            painter.setPen(QPen(QColor("#172337"),3/zoom_)); painter.setBrush(Qt::NoBrush); painter.drawEllipse(cursor,radius_,radius_);
            painter.setPen(QPen(QColor("#ebf2ff"),1/zoom_)); painter.drawEllipse(cursor,radius_,radius_);
        }
        if (dragging_ && maskTarget_ && tool_ == MaskTool::Gradient) {
            painter.setPen(QPen(QColor("#a9c2ff"), 1 / zoom_));
            painter.drawLine(start_, last_);
        }
        if(!maskTarget_ && selections().size()<=1 && (drawingTool_==Tool::Gradient || drawingTool_==Tool::Move)) for(const auto& stored:document_.state().layers) if(stored.id==selected_ && (isGradient(stored.effect) || stored.gradientFill)) {
            const auto& layer=workingEffect_ ? *workingEffect_ : stored;
            const auto endpoints=gradientEndpoints(layer,seconds_); const auto a=endpoints.first,b=endpoints.second;
            painter.setPen(QPen(QColor("#101722"),4/zoom_)); painter.drawLine(a,b);
            painter.setPen(QPen(QColor("#e8f0ff"),2/zoom_)); painter.drawLine(a,b);
            const auto direction=(b-a)/std::max(1.0,QLineF(a,b).length()); const QPointF side(-direction.y(),direction.x());
            painter.drawLine(b,b-direction*(13/zoom_)+side*(6/zoom_)); painter.drawLine(b,b-direction*(13/zoom_)-side*(6/zoom_));
            painter.setBrush(layer.color); painter.drawEllipse(a,6/zoom_,6/zoom_);
            painter.setBrush(layer.secondaryColor); painter.drawEllipse(b,6/zoom_,6/zoom_);
        }
        if(!maskTarget_ && drawingTool_==Tool::Move && selections().size()>1) {
            QRectF bounds;for(const auto& layer:document_.state().layers)if(selections().contains(layer.id))bounds=bounds.united(effectBounds(transformedSelection(layer)));
            painter.setPen(QPen(QColor("#55deff"),1/zoom_));painter.setBrush(Qt::NoBrush);painter.drawRect(bounds);
            for(const auto point:{bounds.topLeft(),bounds.topRight(),bounds.bottomLeft(),bounds.bottomRight()})painter.drawRect(QRectF(point-QPointF(4/zoom_,4/zoom_),QSizeF(8/zoom_,8/zoom_)));
            const auto top=QPointF(bounds.center().x(),bounds.top());painter.drawLine(top,top-QPointF(0,24/zoom_));painter.drawEllipse(top-QPointF(0,24/zoom_),5/zoom_,5/zoom_);
        }
        if (!maskTarget_ && drawingTool_==Tool::Move && selections().size()<=1) for (const auto& stored : document_.state().layers) if (stored.id==selected_ && stored.effect!=EffectKind::StaticColor) {
            const auto& layer=workingEffect_ ? *workingEffect_ : stored;
            const auto transform=effectTransform(layer,seconds_);
            const QRectF bounds(-0.5,-0.5,1,1);
            painter.setPen(QPen(cropTool_ ? Qt::cyan : QColor("#a9c2ff"),1/zoom_)); painter.setBrush(Qt::NoBrush);
            painter.drawPolygon(transform.map(QPolygonF(bounds)));
            if (cropTool_ && dragging_) {
                const auto local=transform.inverted();
                painter.drawPolygon(transform.map(QPolygonF(QRectF(local.map(start_),local.map(last_)).normalized())));
            }
            else if (!cropTool_) {
                for (auto point : {bounds.topLeft(),bounds.topRight(),bounds.bottomRight(),bounds.bottomLeft()}) {
                    const auto handle=transform.map(point); painter.drawRect(QRectF(handle-QPointF(4/zoom_,4/zoom_),QSizeF(8/zoom_,8/zoom_)));
                }
                const auto top=transform.map(QPointF(0,-0.5));
                const auto rotation=transform.map(QPointF(0,-0.5-24/(zoom_*layer.extent.height())));
                painter.drawLine(top,rotation); painter.drawEllipse(rotation,5/zoom_,5/zoom_);
            }
        }
        for(const auto& layer:document_.state().layers)if(layer.id!=selected_ && selections().contains(layer.id)) {
            painter.setPen(QPen(QColor("#55deff"),1/zoom_));painter.setBrush(Qt::NoBrush);painter.drawRect(effectBounds(transformedSelection(layer)));
        }
        if(marquee_){painter.setPen(QPen(QColor("#55deff"),1/zoom_,Qt::DashLine));painter.setBrush(QColor(85,222,255,30));painter.drawRect(QRectF(start_,last_).normalized());}
        for(const auto& line:guides_){painter.setPen(QPen(QColor("#55deff"),1/zoom_,Qt::DashLine));painter.drawLine(line);}
        paintMinimap(painter);return;
    }
    // The same live raster explains sampling positions; Layout edits placements only.
    if(showEffects_) painter.drawImage(world,frame);
    if(!previewOnly_) SnapGrid::load().paint(painter,world.intersected(QRectF(canvasPoint({0,0}),canvasPoint({double(width()),double(height())}))),zoom_);
    if(!previewOnly_) painter.setClipRect(QRectF(canvasPoint(contentRect().topLeft()),canvasPoint(contentRect().bottomRight())));
    if(showComponents_) paintComponents(painter,frame,world);
    if(marquee_){painter.setPen(QPen(QColor("#55deff"),1/zoom_,Qt::DashLine));painter.setBrush(QColor(85,222,255,30));painter.drawRect(QRectF(start_,last_).normalized());}
    for(const auto& line:guides_){painter.setPen(QPen(QColor("#55deff"),1/zoom_,Qt::DashLine));painter.drawLine(line);}
    paintMinimap(painter);
}

void LightingCanvas::paintComponents(QPainter& painter,const QImage& frame,const QRectF& world)
{
    painter.setRenderHint(QPainter::Antialiasing);
    const auto placements=previewPlacements();
    auto sampled=document_.sampleComponents(frame,&placements);
    const auto channels=document_.sampleChannels(frame,seconds_,&placements);
    for(const auto& assignment:document_.state().assignments) {
        int index=0;
        for(const auto& id:assignment.placements) {
            if(assignment.software) {
                const auto count=sampled[id].size();const auto colors=channels.value(assignment.id);
                sampled[id]=colors.mid(index,count);index+=count;
            }
            else {
                QColor color=Qt::black;
                if(assignment.native.id!="off" && !assignment.native.colors.isEmpty()) { const auto rgb=assignment.native.colors.first(); color=QColor(rgb.r,rgb.g,rgb.b); }
                if(sampled.contains(id)) sampled[id].fill(color);
            }
        }
    }
    for (auto placement : document_.state().placements) {
        placement=previewPlacement(placement);
        const auto points = componentPoints(placement);
        const auto& definition = document_.catalog()[placement.componentId];
        const auto body=componentBounds(placement);
        // Project the housing through the same placement transform as its LEDs, including rotation.
        painter.save();painter.translate(placement.origin);painter.rotate(placement.rotation);
        painter.scale(20*placement.scale.width(),20*placement.scale.height());
        drawHardwareComponentArtwork(painter,definition);
        painter.restore();
        const bool keyboard=hardwareCategory(definition)=="Keyboards" && definition.positions.size()-definition.nonPhysicalSlots.size()>20;
        const double ledRadius=keyboard ? 6. : hardwarePreviewLedRadius(definition,placement.scale);
        painter.setPen(QPen(selectedPlacement(placement.id) ? QColor("#a9c2ff") : Qt::white,
                            std::min(1.5/zoom_,ledRadius*.3)));
        const auto keyBody=hardwareBodyBounds(definition);
        const QSizeF keySize(20*placement.scale.width()*keyBody.width()*.2/6.75,
                            20*placement.scale.height()*keyBody.height()*.2/2.25);
        QSet<QPair<int,int>> drawnKeys;
        for (int i = 0; i < points.size(); ++i) {
            if (definition.nonPhysicalSlots.contains(definition.mapping[i])) continue;
            const auto point = points[i];
            if(keyboard) { const auto key=qMakePair(qRound(point.x()*10),qRound(point.y()*10)); if(drawnKeys.contains(key)) continue; drawnKeys.insert(key); }
            if (!world.adjusted(-20, -20, 20, 20).contains(point)) continue;
            const QColor light=sampled[placement.id].value(definition.mapping[i],Qt::black);
            painter.setBrush(light);
            if(keyboard) {
                painter.save();painter.translate(point);painter.rotate(placement.rotation);
                painter.drawRoundedRect(QRectF(-keySize.width()/2,-keySize.height()/2,keySize.width(),keySize.height()),.8,.8);painter.restore();
            } else {
                painter.drawEllipse(point,ledRadius,ledRadius);
                if(ledRadius*zoom_*devicePixelRatioF()<2.) {
                    // Subpixel circles blend with their outline; keep the sampled center color exact when zoomed out.
                    const double dpr=devicePixelRatioF();const auto pixel=(viewportPoint(point)*dpr).toPoint();
                    painter.save();painter.resetTransform();painter.setRenderHint(QPainter::Antialiasing,false);
                    painter.fillRect(QRectF(QPointF(pixel)/dpr,QSizeF(1/dpr,1/dpr)),light);painter.restore();
                }
            }

        }
    }
    // Paint labels and dashed groups after artwork, so filled housings cannot cover them.
    if(!previewOnly_ && context_==Context::Layout) for(const auto& assignment:document_.state().assignments) {
        if(!visibleChain(assignment))continue;
        const auto bounds=groupBounds(assignment);
        if(bounds.isEmpty()) continue;
        painter.setBrush(Qt::NoBrush); painter.setPen(QPen(QColor("#8196b8"),1/zoom_,Qt::DashLine));
        painter.drawRoundedRect(bounds,8,8);
        const auto title=groupLabel(assignment);
        painter.fillRect(title,QColor("#202735")); painter.setPen(QColor("#dce7ff")); painter.drawText(title,Qt::AlignCenter,assignment.channelId);
    }
    if(!previewOnly_ && context_==Context::Layout) for(const auto& placement:placements) {
        bool chained=false;for(const auto& assignment:document_.state().assignments)
            if(visibleChain(assignment) && assignment.placements.contains(placement.id))chained=true;
        if(!chained) {
            painter.setBrush(Qt::NoBrush);painter.setPen(QPen(QColor("#8196b8"),1/zoom_,Qt::DashLine));
            painter.drawRoundedRect(componentBounds(placement).adjusted(-8,-8,8,8),8,8);
        }
        if(showLabels_) {
            const auto label=componentLabel(placement);
            painter.save();painter.setFont(componentLabelFont());
            painter.fillRect(label,QColor("#202735"));painter.setPen(QColor("#dce7ff"));
            painter.drawText(label,Qt::AlignLeft|Qt::AlignVCenter,componentLabelText(placement));painter.restore();
        }
    }
    if(!previewOnly_ && context_==Context::Layout && !selected_.isNull()) {
        auto bounds=selectionBounds();

        painter.setPen(QPen(QColor("#a9c2ff"),1.5/zoom_)); painter.setBrush(Qt::NoBrush);
        painter.drawRect(bounds);
        for(const auto point:{bounds.topLeft(),bounds.topRight(),bounds.bottomLeft(),bounds.bottomRight()}) {
            painter.setBrush(QColor("#202838")); painter.drawRect(QRectF(point-QPointF(5/zoom_,5/zoom_),QSizeF(10/zoom_,10/zoom_)));
        }
        const auto [top,rotation]=rotationHandle();
        painter.drawLine(top,rotation); painter.drawEllipse(rotation,6/zoom_,6/zoom_);
    }
}

QRectF LightingCanvas::navigationBounds() const
{
    const auto placements=previewPlacements();
    if(context_==Context::Layout) {
        auto bounds=document_.layoutBounds(&placements);
        for(const auto& placement:placements) {
            bounds=bounds.united(componentBounds(placement));
            // Reserve caption space even while hidden so holding the reveal key never shifts the camera.
            bounds=bounds.united(componentLabel(placement));
        }
        // Group handles and labels must remain reachable above components at the outermost edge.
        for(const auto& assignment:document_.state().assignments) if(visibleChain(assignment) && !groupBounds(assignment).isEmpty())
            bounds=bounds.united(groupBounds(assignment)).united(groupLabel(assignment));
        return bounds;
    }
    // Effects is a fixed raster; wrapped/animated sources outside it do not enlarge its navigation area.
    return QRectF(0,0,640,400);
}
void LightingCanvas::syncScrollbars()
{
    if(previewOnly_){horizontalScroll_->hide();verticalScroll_->hide();return;}
    const auto world=navigationBounds();
    const auto scale=displayScale();
    const int thickness=style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    // Reserve the header on all zoom levels; scrollbar geometry is independent of content painting.
    bool horizontal=world.width()*scale.width()*zoom_>width()-32;
    bool vertical=world.height()*scale.height()*zoom_>height()-64-(horizontal ? thickness : 0);
    horizontal=world.width()*scale.width()*zoom_>width()-32-(vertical ? thickness : 0);
    vertical=world.height()*scale.height()*zoom_>height()-64-(horizontal ? thickness : 0);
    horizontalScroll_->setVisible(horizontal);verticalScroll_->setVisible(vertical);
    const auto area=contentRect();
    const int visibleWidth=int(area.width()),visibleHeight=int(area.height());
    horizontalScroll_->setGeometry(16,height()-16-thickness,visibleWidth,thickness);
    verticalScroll_->setGeometry(width()-16-thickness,48,thickness,visibleHeight);
    auto origin=viewportPoint(world.topLeft());
    if(!dragging_ && !panning_ && !navigating_) {
        // A clamped thumb must describe the actual camera; an already-maximal value emits no change signal.
        // Defer correction during gestures so growing bounds never move the canvas under the pointer.
        if(horizontal)pan_.rx()+=std::clamp(origin.x(),area.right()-world.width()*scale.width()*zoom_,area.left())-origin.x();
        if(vertical)pan_.ry()+=std::clamp(origin.y(),area.bottom()-world.height()*scale.height()*zoom_,area.top())-origin.y();
        origin=viewportPoint(world.topLeft());
    }
    const auto sync=[](QScrollBar* bar,double extent,int visible,double origin) {
        // Bound the integer scrollbar range even for very large legacy coordinates; preserve double world positions.
        const double unit=std::max(1.0,extent/1000000.0);
        const QSignalBlocker block(bar);
        bar->setRange(0,int(std::clamp(std::ceil((extent-visible)/unit),0.0,1000000.0)));
        bar->setPageStep(std::max(1,int(std::ceil(visible/unit))));
        bar->setValue(int(std::clamp(std::round(-origin/unit),0.0,double(bar->maximum()))));
        return unit;
    };
    scrollUnits_={sync(horizontalScroll_,world.width()*scale.width()*zoom_,visibleWidth,origin.x()-area.left()),
                  sync(verticalScroll_,world.height()*scale.height()*zoom_,visibleHeight,origin.y()-area.top())};
    fullscreen_->move(width()-48-(vertical ? thickness : 0),10);
}
QRectF LightingCanvas::minimapRect() const {
    if(previewOnly_ || (horizontalScroll_->isHidden() && verticalScroll_->isHidden())) return {};
    // Reserve both scrollbar gutters even when only one is visible, keeping the overlay stationary.
    const int gutter=style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    return QRectF(width()-16-gutter-188,height()-16-gutter-122,176,110);
}
void LightingCanvas::paintMinimap(QPainter& painter)
{
    if(minimapRect().isEmpty())return;
    painter.save();painter.resetTransform();painter.setClipping(false);
    const auto box=minimapRect();const auto world=navigationBounds();
    painter.fillRect(box,QColor("#10151f"));
    const auto map=[&](QPointF point){return box.topLeft()+QPointF((point.x()-world.left())/world.width()*box.width(),(point.y()-world.top())/world.height()*box.height());};
    const auto placements=previewPlacements();
    const auto rasterBounds=context_==Context::Layout ? document_.layoutBounds(&placements) : QRectF(0,0,640,400);
    const auto image=QRectF(map(rasterBounds.topLeft()),map(rasterBounds.bottomRight()));
    painter.drawImage(image,rendered_);painter.setPen(QColor("#e6f1ff"));
    if(context_==Context::Layout)for(const auto& placement:previewPlacements())for(const auto& point:componentPoints(placement))painter.drawEllipse(map(point),2,2);
    const auto viewport=QRectF(map(canvasPoint(contentRect().topLeft())),map(canvasPoint(contentRect().bottomRight()))).intersected(box);
    painter.setBrush(Qt::NoBrush);painter.setPen(QPen(QColor("#93bfff"),2));painter.drawRect(viewport);
    painter.setPen(QColor("#596981"));painter.drawRect(box);painter.restore();
}
void LightingCanvas::navigateMinimap(QPointF position)
{
    const auto box=minimapRect(),world=navigationBounds();
    const QPointF target(world.left()+std::clamp((position.x()-box.left())/box.width(),0.0,1.0)*world.width(),
                         world.top()+std::clamp((position.y()-box.top())/box.height(),0.0,1.0)*world.height());
    autoFit_=false;pan_+=QPointF(width()/2.0,height()/2.0)-viewportPoint(target);update();
}

QRectF LightingCanvas::componentBounds(const ComponentPlacement& placement) const
{
    const auto& definition=document_.catalog()[placement.componentId];
    QTransform transform;transform.translate(placement.origin.x(),placement.origin.y());
    transform.rotate(placement.rotation);transform.scale(20*placement.scale.width(),20*placement.scale.height());
    QRectF leds;const auto points=componentPoints(placement);
    // Reserved protocol slots must not enlarge a device's frame or the focused hardware camera.
    for(int i=0;i<points.size();++i)if(!definition.nonPhysicalSlots.contains(definition.mapping.value(i)))leds=leds.united(QRectF(points[i],QSizeF(.01,.01)));
    return transform.mapRect(hardwareBodyBounds(definition)).united(leds.adjusted(-10,-10,10,10));
}
QFont LightingCanvas::componentLabelFont() const
{
    auto labelFont=font();labelFont.setPointSizeF(std::max(7.,labelFont.pointSizeF()*.75));return labelFont;
}

QRectF LightingCanvas::componentLabel(const ComponentPlacement& placement) const
{
    const auto body=componentBounds(placement);const QFontMetrics metrics(componentLabelFont());
    // A fixed lower-left anchor never moves to avoid other components; tooltips carry the full name.
    return {body.left(),body.bottom()+10.,metrics.horizontalAdvance(componentLabelText(placement))+8.,metrics.height()+2.};
}

QString LightingCanvas::componentLabelText(const ComponentPlacement& placement) const
{
    return QFontMetrics(componentLabelFont()).elidedText(document_.componentName(placement),Qt::ElideRight,
        int(std::max(100.,componentBounds(placement).width())));
}

QPair<QPointF,QPointF> LightingCanvas::rotationHandle() const
{
    const auto bounds=selectionBounds();
    double angle=0;
    for(const auto& placement:document_.state().placements)if(selectedPlacement(placement.id)) {
        angle=previewPlacement(placement).rotation;break;
    }
    QTransform rotation;rotation.rotate(angle);
    const auto direction=rotation.map(QPointF(0,-1));
    // Intersect the rotated direction with the selection so the grip follows rotation outside its body.
    const double distance=std::min(bounds.width()/(2*std::max(1e-12,std::abs(direction.x()))),
                                   bounds.height()/(2*std::max(1e-12,std::abs(direction.y()))));
    const auto top=bounds.center()+direction*distance;
    return {top,top+direction*(24/zoom_)};
}

QRectF LightingCanvas::selectionBounds() const
{
    QRectF bounds,housings;
    if(context_==Context::Effects){for(const auto& layer:document_.state().layers)if(selections().contains(layer.id))bounds=bounds.united(effectBounds(layer));return bounds;}
    for(const auto& placement:document_.state().placements)if(selectedPlacement(placement.id)) {
        const auto preview=previewPlacement(placement);
        for(const auto& point:componentPoints(preview))bounds=bounds.united(QRectF(point,QSizeF(.01,.01)));
        housings=housings.united(componentBounds(preview));
    }
    return bounds.isEmpty() ? QRectF() : bounds.adjusted(-15,-15,15,15).united(housings.adjusted(-5,-5,5,5));
}
QVector<ComponentPlacement> LightingCanvas::previewPlacements() const
{
    auto placements=document_.state().placements;
    if(context_==Context::Layout) for(auto& placement:placements) placement=previewPlacement(placement);
    return placements;
}

ComponentPlacement LightingCanvas::previewPlacement(ComponentPlacement placement) const
{
    if(!dragging_ || !selectedPlacement(placement.id)) return placement;
    if(handle_==0) placement.origin+=last_-start_;
    else {
        QTransform transform; transform.translate(layoutAnchor_.x(),layoutAnchor_.y());
        transform.rotate(layoutRotation_); transform.scale(layoutScale_,layoutScale_);
        transform.translate(-layoutAnchor_.x(),-layoutAnchor_.y());
        placement.origin=transform.map(placement.origin);
        placement.scale*=layoutScale_;
        placement.rotation=std::remainder(placement.rotation+layoutRotation_,360.0);
    }
    return placement;
}

QPointF LightingCanvas::paintPoint(QPointF point) const
{
    if(maskTarget_) return point;
    const auto local=effectTransform(*originalEffect_,seconds_).inverted().map(point)+QPointF(.5,.5);
    return {local.x()*640,local.y()*400};
}

void LightingCanvas::beginDrawing(QPointF point)
{
    EffectLayer layer; bool found=false;
    for(const auto& candidate:document_.state().layers) if(candidate.id==selected_) { layer=candidate; found=true; }
    if(found && !editable(layer.id)){emit editingError("Selection is locked. Unlock it before editing.");return;}
    if(!found && drawingTool_==Tool::Eraser) return;
    if(found && !maskTarget_ && drawingTool_==Tool::Eraser && layer.effect!=EffectKind::Paint) {
        // Procedural sources stay editable: erasing edits their existing mask instead of flattening pixels.
        emit maskEditingRequested(); maskTarget_=true;
    }
    if(maskTarget_ && !found) return;
    if(found && layer.container && drawingTool_==Tool::Eraser){emit editingError("Select an object to erase its pixels or mask.");return;}
    const bool gradient=drawingTool_==Tool::Gradient;
    const bool solid=layer.effect==EffectKind::StaticColor || (isShape(layer.effect));
    if(found && !maskTarget_ && drawingTool_==Tool::Fill && solid && renderEffect(layer).pixelColor(point.toPoint()).alpha()>0) {
        originalEffect_=workingEffect_=layer; workingEffect_->color=drawColor_; workingEffect_->gradientFill=false; dragging_=true; handle_=9; return;
    }
    // Bucket fills use the selected procedural source as their boundary, while keeping that source editable below the paint result.
    const auto reference=!maskTarget_ && found && drawingTool_==Tool::Fill && layer.effect!=EffectKind::Paint ? renderEffect(layer) : QImage{};
    if(!maskTarget_ && (gradient || !found || layer.container || (!layer.parentId.isNull() && (drawingTool_==Tool::Rectangle || drawingTool_==Tool::Ellipse || drawingTool_==Tool::Line)) || (gradient ? !isGradient(layer.effect) && layer.effect!=EffectKind::Paint && !solid : layer.effect!=EffectKind::Paint))) {
        if(document_.state().layers.size()>=64) { emit editingError("Layer limit reached. Select an existing paint layer or remove a layer first."); return; }
        layer=EffectLayer{};layer.parentId=selected_.isNull() ? activeLayer_ : document_.layerRoot(selected_);layer.effect=gradient ? EffectKind::LinearGradient : EffectKind::Paint;
        layer.name=gradient ? "Gradient" : "Paint";if(gradient){layer.gradientColors=gradientColors_;layer.gradientPositions=gradientPositions_;layer.color=drawColor_;layer.secondaryColor=Qt::black;}layer.mask=LightingDocument::whiteMask();layer.extent=LightingDocument::canvasSize();createdObject_=true;
    }
    if(createdObject_ && filled_ && (drawingTool_==Tool::Rectangle || drawingTool_==Tool::Ellipse)) {
        layer.effect=drawingTool_==Tool::Rectangle ? EffectKind::Rectangle : EffectKind::Circle;layer.name=effectDefinition(layer.effect).label;layer.color=drawColor_;
    }
    originalEffect_=workingEffect_=layer; fillReference_=reference; drawing_=dragging_=true; start_=last_=point;
    if(maskTarget_) drawingBase_=layer.mask.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    else {
        drawingBase_=layer.image;
        if(drawingBase_.isNull()) { drawingBase_=QImage(LightingDocument::canvasSize(),QImage::Format_ARGB32_Premultiplied); drawingBase_.fill(Qt::transparent); }
    }
    updateDrawing(point); update();
}

void LightingCanvas::updateDrawing(QPointF point)
{
    if(!originalEffect_) return;
    const bool gradient=drawingTool_==Tool::Gradient;
    if(createdObject_ && isShape(originalEffect_->effect) && (drawingTool_==Tool::Rectangle || drawingTool_==Tool::Ellipse)) {
        workingEffect_->center=(start_+point)/2;workingEffect_->extent={std::max(1.0,std::abs(point.x()-start_.x())),std::max(1.0,std::abs(point.y()-start_.y()))};return;
    }
    if(gradient && !maskTarget_ && isShape(originalEffect_->effect)) {
        workingEffect_->gradientFill=true; workingEffect_->gradientStart=QPointF(paintPoint(start_).x()/640,paintPoint(start_).y()/400); workingEffect_->gradientEnd=QPointF(paintPoint(point).x()/640,paintPoint(point).y()/400);
        workingEffect_->gradientColors=gradientColors_.isEmpty() ? QVector<QColor>{drawColor_,Qt::black} : gradientColors_; workingEffect_->gradientPositions=gradientPositions_; return;
    }
    if(gradient && !maskTarget_ && !(originalEffect_->effect==EffectKind::Paint && !originalEffect_->image.isNull())) {
        workingEffect_->effect=isGradient(originalEffect_->effect) ? originalEffect_->effect : EffectKind::LinearGradient;
        const bool centered=centeredGradient(workingEffect_->effect);
        workingEffect_->center=centered ? start_ : (start_+point)/2;
        workingEffect_->center.setX(std::clamp(workingEffect_->center.x(),-10000.0,10000.0)); workingEffect_->center.setY(std::clamp(workingEffect_->center.y(),-10000.0,10000.0));
        workingEffect_->extent={std::clamp(QLineF(start_,point).length()*(centered ? 2 : 1),1.0,10000.0),centered ? std::clamp(QLineF(start_,point).length()*2,1.0,10000.0) : 400};
        workingEffect_->rotation=std::atan2(point.y()-start_.y(),point.x()-start_.x())*180/3.141592653589793;
        if(originalEffect_->effect==EffectKind::Paint) { workingEffect_->color=drawColor_; workingEffect_->secondaryColor=Qt::black; }
        return;
    }
    const bool freehand=drawingTool_==Tool::Pencil || drawingTool_==Tool::Eraser;
    if(freehand && maskTarget_) {
        if(workingMask_.isNull()) workingMask_=originalEffect_->mask;
        LightingDocument::paintMask(workingMask_,last_,point,drawWidth_/2,hardness_,opacity_,drawingTool_==Tool::Eraser ? 0 : gray_); return;
    }
    auto image=freehand && !workingEffect_->image.isNull() && !maskTarget_ ? workingEffect_->image :
        freehand && !workingMask_.isNull() && maskTarget_ ? workingMask_.convertToFormat(QImage::Format_ARGB32_Premultiplied) : drawingBase_;
    const QPointF from=paintPoint(freehand ? last_ : start_), to=paintPoint(point);
    QColor color=maskTarget_ ? QColor(gray_,gray_,gray_) : drawColor_;
    if(maskTarget_ && drawingTool_==Tool::Eraser) color=Qt::black;
    if(gradient && !maskTarget_) {
        QLinearGradient ramp(paintPoint(start_),paintPoint(point));
        const auto colors=gradientColors_.isEmpty() ? QVector<QColor>{drawColor_,Qt::black} : gradientColors_;
        for(int i=0;i<colors.size();++i) ramp.setColorAt(gradientPositions_.value(i,i/double(colors.size()-1)),colors[i]);
        // SourceIn recolors only existing coverage, retaining every antialiased edge pixel.
        QPainter painter(&image); painter.setCompositionMode(QPainter::CompositionMode_SourceIn); painter.fillRect(image.rect(),ramp);
    } else if(drawingTool_==Tool::Fill) {
        const QPoint seed=to.toPoint();
        if(image.rect().contains(seed)) {
            const auto reference=fillReference_.isNull() ? image : fillReference_;
            const QRgb before=reference.pixel(seed), after=qPremultiply(color.rgba());
            if(before!=after || image.pixel(seed)!=after) {
                // Mark on enqueue: a flood fill never retains duplicate pixels or recurses on the UI stack.
                const auto matches=[&](QRgb pixel) {
                    if(qAlpha(before)==0) return qAlpha(pixel)==0;
                    if(qAlpha(pixel)==0) return false;
                    const auto a=qUnpremultiply(pixel), b=qUnpremultiply(before);
                    const int tolerance=24+255/qAlpha(pixel);
                    return std::abs(qRed(a)-qRed(b))<=tolerance && std::abs(qGreen(a)-qGreen(b))<=tolerance && std::abs(qBlue(a)-qBlue(b))<=tolerance;
                };
                QImage visited(image.size(),QImage::Format_Grayscale8); visited.fill(0);
                const auto replacement=[&](QPoint at) {
                    auto value=color; value.setAlpha(fillReference_.isNull() && qAlpha(before)>0 ? qAlpha(reference.pixel(at)) : color.alpha());
                    return qPremultiply(value.rgba());
                };
                QVector<QPoint> pending{seed}; image.setPixel(seed,replacement(seed)); visited.scanLine(seed.y())[seed.x()]=1;
                for(qsizetype i=0;i<pending.size();++i) for(const auto delta:{QPoint(1,0),QPoint(-1,0),QPoint(0,1),QPoint(0,-1)}) {
                    const auto next=pending[i]+delta;
                    if(image.rect().contains(next) && !visited.constScanLine(next.y())[next.x()] && matches(reference.pixel(next))) { visited.scanLine(next.y())[next.x()]=1; image.setPixel(next,replacement(next)); pending.append(next); }
                }
            }
        }
    } else {
        QPainter painter(&image); painter.setRenderHint(QPainter::Antialiasing);
        if(drawingTool_==Tool::Eraser && !maskTarget_) painter.setCompositionMode(QPainter::CompositionMode_Clear);
        painter.setPen(QPen(color,drawWidth_,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
        painter.setBrush(filled_ ? QBrush(color) : Qt::NoBrush);
        const QRectF bounds=QRectF(from,to).normalized();
        if(gradient) { QLinearGradient ramp(from,to); ramp.setColorAt(0,Qt::black); ramp.setColorAt(1,Qt::white); painter.fillRect(image.rect(),ramp); }
        else if(drawingTool_==Tool::Rectangle) painter.drawRect(bounds);
        else if(drawingTool_==Tool::Ellipse) painter.drawEllipse(bounds);
        else if(from==to) painter.drawPoint(to);
        else painter.drawLine(from,to);
    }
    if(maskTarget_) workingMask_=image.convertToFormat(QImage::Format_Grayscale8);
    else workingEffect_->image=image;
}

bool LightingCanvas::visibleChain(const ChannelAssignment& assignment) const
{
    // Dedicated peripherals have output assignments, but no user-facing component chain.
    for(const auto& placement:document_.state().placements)if(assignment.placements.contains(placement.id)) {
        const auto category=hardwareCategory(document_.catalog()[placement.componentId]);
        if(category!="Mice" && category!="Keyboards" && category!="RAM" && category!="Graphics cards" && category!="Motherboards")return true;
    }
    return false;
}

QRectF LightingCanvas::groupBounds(const ChannelAssignment& assignment) const
{
    QRectF bounds,housings;
    for(auto placement:document_.state().placements)if(assignment.placements.contains(placement.id)) {
        placement=previewPlacement(placement);
        for(const auto& point:componentPoints(placement))bounds=bounds.united(QRectF(point,QSizeF(.01,.01)));
        housings=housings.united(componentBounds(placement));
    }
    return bounds.isEmpty() ? QRectF{} : bounds.adjusted(-18,-25,18,18).united(housings.adjusted(-8,-8,8,8));
}
QRectF LightingCanvas::groupLabel(const ChannelAssignment& assignment) const
{
    const auto bounds=groupBounds(assignment); if(bounds.isEmpty()) return {};
    const QFontMetrics metrics(font());
    return {bounds.left(),bounds.top()-metrics.height()-6,metrics.horizontalAdvance(assignment.channelId)+12.,metrics.height()+4.};
}

bool LightingCanvas::selectedPlacement(const QUuid& id) const
{
    if(!editable(id))return false;
    if(id==selected_ || selectedIds_.contains(id)) return true;
    for(const auto& assignment:document_.state().assignments)
        if(assignment.id==selected_ || selectedIds_.contains(assignment.id)) return editable(assignment.id) && assignment.placements.contains(id);
    return false;
}

void LightingCanvas::mousePressEvent(QMouseEvent* event)
{
    if(previewOnly_ || (context_==Context::Layout ? !showComponents_ : !showEffects_)) return;
    setFocus();
    if (event->button() == Qt::MiddleButton || (event->button()==Qt::LeftButton && spaceHeld_)) { cancelGesture(); autoFit_=false; panning_ = true; last_ = event->position(); return; }
    if(event->button()==Qt::RightButton) {
        const auto hits=hitsAt(canvasPoint(event->position()));QMenu menu(this);
        for(const auto& id:hits) {
            QString name;
            for(const auto& layer:document_.state().layers)if(layer.id==id)name=layer.name;
            for(const auto& item:document_.state().placements)if(item.id==id)name=document_.componentName(item);
            auto* action=menu.addAction(name);connect(action,&QAction::triggered,this,[this,id]{selectMany({id},id);});
        }
        if(!hits.isEmpty())menu.exec(event->globalPosition().toPoint());return;
    }
    if (event->button() != Qt::LeftButton) return;
    if(minimapRect().contains(event->position())){cancelGesture();navigating_=true;navigateMinimap(event->position());return;}
    if(!contentRect().contains(event->position())) return;
    pointer_=event->position(); pointerModifiers_=event->modifiers();
    start_ = last_ = canvasPoint(event->position());
    snapGesture_=SnapGrid::load().enabled && !(event->modifiers() & Qt::AltModifier);
    if (context_==Context::Effects && !QRectF(QPointF(), LightingDocument::canvasSize()).contains(start_)) return;
    if((event->modifiers() & Qt::ShiftModifier) && (context_==Context::Layout || (!maskTarget_ && drawingTool_==Tool::Move))) {
        const auto hits=hitsAt(start_);auto ids=selectedIds_;
        if(!hits.isEmpty()){const auto id=hits.first();if(ids.contains(id))ids.remove(id);else ids.insert(id);selectMany(ids,ids.contains(id) ? id : QUuid{});return;}
    }
    if((context_==Context::Layout || (!maskTarget_ && drawingTool_==Tool::Move)) && hitsAt(start_).isEmpty()) {
        // An existing selection's grips remain reachable outside painted pixels.
        bool onGrip=false;
        if(context_==Context::Effects && selections().size()>1){QRectF bounds;for(const auto& layer:document_.state().layers)if(selections().contains(layer.id))bounds=bounds.united(effectBounds(layer));for(const auto point:{bounds.topLeft(),bounds.topRight(),bounds.bottomLeft(),bounds.bottomRight(),QPointF(bounds.center().x(),bounds.top()-24/zoom_)})onGrip|=QLineF(start_,point).length()<12/zoom_;}
        if(context_==Context::Layout && !selectionBounds().isEmpty()) {
            const auto bounds=selectionBounds();for(const auto point:{bounds.topLeft(),bounds.topRight(),bounds.bottomLeft(),bounds.bottomRight(),rotationHandle().second})onGrip|=QLineF(start_,point).length()<12/zoom_;
        } else for(const auto& layer:document_.state().layers)if(layer.id==selected_ && editable(layer.id)) {
            const auto transform=effectTransform(layer,seconds_);for(const auto point:{QPointF(-.5,-.5),QPointF(.5,-.5),QPointF(-.5,.5),QPointF(.5,.5),QPointF(0,-.5-24/(zoom_*layer.extent.height()))})onGrip|=QLineF(start_,transform.map(point)).length()<12/zoom_;
            if(isGradient(layer.effect) || layer.gradientFill){const auto ends=gradientEndpoints(layer,seconds_);onGrip|=QLineF(start_,ends.first).length()<12/zoom_ || QLineF(start_,ends.second).length()<12/zoom_;}
        }
        bool groupLabelHit=false;for(const auto& assignment:document_.state().assignments)if(context_==Context::Layout && visibleChain(assignment) && groupLabel(assignment).contains(start_))groupLabelHit=true;
        if(context_==Context::Layout && !selected_.isNull() && selectionBounds().contains(start_))onGrip=true;
        if(!onGrip && !groupLabelHit){marquee_=true;marqueeBefore_=selectedIds_;additiveMarquee_=event->modifiers() & Qt::ShiftModifier;return;}
    }
    if(context_==Context::Effects && !maskTarget_ && drawingTool_!=Tool::Pencil && drawingTool_!=Tool::Fill && drawingTool_!=Tool::Eraser) {
        if(drawingTool_==Tool::Gradient || drawingTool_==Tool::Move) for(const auto& layer:document_.state().layers)
            if(layer.id==selected_ && editable(layer.id) && (isGradient(layer.effect) || layer.gradientFill)) {
                const auto ends=gradientEndpoints(layer,seconds_);
                if(QLineF(start_,ends.first).length()<12/zoom_ || QLineF(start_,ends.second).length()<12/zoom_) {
                    emit effectGestureStarted();handle_=QLineF(start_,ends.first).length()<12/zoom_ ? 7 : 8;
                    originalEffect_=workingEffect_=layer;dragging_=true;return;
                }
            }
        if(drawingTool_!=Tool::Move){pendingDrawing_=true;return;}
        bool onHandle=false;
        if(selections().size()>1){const auto bounds=selectionBounds();for(const auto point:{bounds.topLeft(),bounds.topRight(),bounds.bottomLeft(),bounds.bottomRight(),QPointF(bounds.center().x(),bounds.top()-24/zoom_)})onHandle|=QLineF(start_,point).length()<12/zoom_;}
        for(const auto& layer:document_.state().layers)if(layer.id==selected_ && editable(layer.id) && !layer.container) {
            const auto transform=effectTransform(layer,seconds_);
            for(const auto corner:{QPointF(-.5,-.5),QPointF(.5,-.5),QPointF(.5,.5),QPointF(-.5,.5),QPointF(0,-.5-24/(zoom_*layer.extent.height()))})
                onHandle|=QLineF(start_,transform.map(corner)).length()<12/zoom_;
        }
        if(!onHandle) {
            const auto hits=hitsAt(start_);
            // Drag a selection chosen in the tree without allowing a covering source to steal it.
            bool inside=false;for(const auto& layer:document_.state().layers)if(selectedIds_.contains(layer.id) && editable(layer.id) && effectBounds(layer).contains(start_))inside=true;
            deferredHit_=inside && !hits.isEmpty() && hits.first()!=selected_;
            if(!inside)selectAt(start_);
        }
    }
    if(context_==Context::Effects && !editable(selected_) && !selected_.isNull()){emit editingError("Selection is locked.");return;}
    if(context_==Context::Effects) emit effectGestureStarted();
    if(context_==Context::Effects && !maskTarget_ && drawingTool_==Tool::Move && (selections().size()>1 || !selections().contains(selected_))) {
        QRectF bounds;for(const auto& layer:document_.state().layers)if(selections().contains(layer.id))bounds=bounds.united(effectBounds(layer));
        if(!bounds.isEmpty()) {
            int handle=0;const QVector<QPointF> corners{bounds.topLeft(),bounds.topRight(),bounds.bottomRight(),bounds.bottomLeft()};
            for(int i=0;i<corners.size();++i)if(QLineF(start_,corners[i]).length()<12/zoom_)handle=i+1;
            if(QLineF(start_,QPointF(bounds.center().x(),bounds.top()-24/zoom_)).length()<12/zoom_)handle=5;
            if(!handle && bounds.contains(start_))handle=6;
            if(handle){EffectLayer group;group.center=bounds.center();group.extent=bounds.size();group.effect=EffectKind::Rectangle;originalEffect_=workingEffect_=group;handle_=handle;dragging_=true;return;}
        }
    }
    if(context_==Context::Effects && drawingTool_!=Tool::Move) {
        if(drawingTool_==Tool::Gradient && !maskTarget_) for(const auto& layer:document_.state().layers) if(layer.id==selected_ && editable(layer.id) && (isGradient(layer.effect) || layer.gradientFill)) {
            const auto endpoints=gradientEndpoints(layer,seconds_); const auto a=endpoints.first,b=endpoints.second;
            if(QLineF(start_,a).length()<12/zoom_ || QLineF(start_,b).length()<12/zoom_) {
                handle_=QLineF(start_,a).length()<12/zoom_ ? 7 : 8;
                originalEffect_=workingEffect_=layer; dragging_=true; return;
            }
        }
        if(snapGesture_ && drawingTool_!=Tool::Pencil && drawingTool_!=Tool::Eraser && drawingTool_!=Tool::Fill)
            start_=last_=SnapGrid::load().snap(start_);
        beginDrawing(start_); return;
    }
    if (context_ == Context::Layout) {
        // Hit the exact painted label before member bodies or handles; it always moves the complete chain.
        for(auto it=document_.state().assignments.crbegin();it!=document_.state().assignments.crend();++it)
            if(editable(it->id) && visibleChain(*it) && groupLabel(*it).contains(start_)) { select(it->id); layoutBounds_=selectionBounds(); handle_=0; dragging_=true; return; }
        bool groupSelected=false;
        for(const auto& assignment:document_.state().assignments) if(assignment.id==selected_) groupSelected=true;
        if(groupSelected) for(auto it=document_.state().placements.crbegin();it!=document_.state().placements.crend();++it) {
            if(editable(it->id) && (componentBounds(*it).contains(start_) || (showLabels_ && componentLabel(*it).contains(start_)))) {
                select(it->id); layoutBounds_=selectionBounds(); handle_=0; dragging_=true; return;
            }
        }
        layoutBounds_=selectionBounds();
        if(!layoutBounds_.isEmpty()) {
            const QVector<QPointF> corners{layoutBounds_.topLeft(),layoutBounds_.topRight(),layoutBounds_.bottomRight(),layoutBounds_.bottomLeft()};
            for(int i=0;i<4;++i) if(QLineF(start_,corners[i]).length()<12/zoom_) {handle_=1;layoutAnchor_=corners[(i+2)%4];}
            if(QLineF(start_,rotationHandle().second).length()<12/zoom_) handle_=5;
            if(handle_) { if(handle_==5) layoutAnchor_=layoutBounds_.center(); dragging_=true; return; }
        }
        QUuid hit;
        for (auto it = document_.state().placements.crbegin(); it != document_.state().placements.crend(); ++it) {
            if(editable(it->id) && (componentBounds(*it).contains(start_) || (showLabels_ && componentLabel(*it).contains(start_)))) { hit=it->id; origin_=it->origin; break; }
            if(!editable(it->id))continue;
            for (auto point : componentPoints(*it))
                if (QLineF(point, start_).length() < 12 / zoom_) { hit = it->id; origin_ = it->origin; break; }
            if (!hit.isNull()) break;
        }
        if(hit.isNull() && !selected_.isNull() && layoutBounds_.contains(start_)) { dragging_=true; return; }
        // The group label moves the chain; a member body always selects that component.
        if(!selectedIds_.contains(hit))select(hit);
        dragging_ = !hit.isNull();
    } else if (!maskTarget_) {
        for (const auto& layer : document_.state().layers) if (layer.id==selected_ && editable(layer.id) && layer.effect!=EffectKind::StaticColor) {
            const auto transform=effectTransform(layer,seconds_);
            const QRectF bounds(-0.5,-0.5,1,1);
            if (cropTool_ && (layer.effect!=EffectKind::Image || layer.image.isNull())) return;
            int handle=0;
            if (!cropTool_) {
                const QVector<QPointF> corners{bounds.topLeft(),bounds.topRight(),bounds.bottomRight(),bounds.bottomLeft()};
                for (int i=0;i<4;++i) if (QLineF(start_,transform.map(corners[i])).length()<10/zoom_) handle=i+1;
                const auto rotation=transform.map(QPointF(0,-0.5-24/(zoom_*layer.extent.height())));
                if (QLineF(start_,rotation).length()<10/zoom_) handle=5;
            }
            if (handle==0 && bounds.contains(transform.inverted().map(start_))) handle=6;
            if (handle) { originalEffect_=layer; workingEffect_=layer; handle_=handle; dragging_=true; }
        }
    } else if (maskTarget_) {
        if(snapGesture_ && tool_==MaskTool::Gradient) start_=last_=SnapGrid::load().snap(start_);
        for (const auto& layer : document_.state().layers) if (layer.id == selected_) {
            workingMask_ = layer.mask;
            dragging_ = true;
            if (tool_ == MaskTool::Brush)
                LightingDocument::paintMask(workingMask_, start_, start_, radius_, hardness_, opacity_, gray_);
        }
    }
    update();
}

void LightingCanvas::mouseMoveEvent(QMouseEvent* event)
{
    if(deferredHit_) {
        if(QLineF(viewportPoint(start_),event->position()).length()<QApplication::startDragDistance())return;
        deferredHit_=false;
    }
    if(marquee_){last_=canvasPoint(event->position());update();return;}
    if(navigating_){navigateMinimap(event->position());return;}
    if (panning_) { pan_ += event->position() - last_; last_ = event->position(); update(); return; }
    if(pendingDrawing_) {
        if(QLineF(viewportPoint(start_),event->position()).length()<QApplication::startDragDistance())return;
        pendingDrawing_=false;emit effectGestureStarted();
        if(drawingTool_==Tool::Gradient)for(const auto& layer:document_.state().layers)if(layer.id==selected_ && editable(layer.id) && (isGradient(layer.effect) || layer.gradientFill)) {
            const auto endpoints=gradientEndpoints(layer,seconds_);
            if(QLineF(start_,endpoints.first).length()<12/zoom_ || QLineF(start_,endpoints.second).length()<12/zoom_) {
                handle_=QLineF(start_,endpoints.first).length()<12/zoom_ ? 7 : 8;originalEffect_=workingEffect_=layer;dragging_=true;
                mouseMoveEvent(event);return;
            }
        }
        if(snapGesture_)start_=last_=SnapGrid::load().snap(start_);
        beginDrawing(start_);
        if(dragging_){updateDrawing(canvasPoint(event->position()));last_=canvasPoint(event->position());update();}return;
    }
    if (!dragging_) {
        if(context_==Context::Layout && !previewOnly_) {
            QString name;const auto point=canvasPoint(event->position());
            for(const auto& placement:document_.state().placements)
                if((showLabels_ && componentLabel(placement).contains(point)) || componentBounds(placement).contains(point))name=document_.componentName(placement);
            setToolTip(name);
        }
        if(maskTarget_) update(); return;
    }
    pointer_=event->position(); pointerModifiers_=event->modifiers();
    if(context_==Context::Layout && !autoScroll_->isActive()) {autoScrollClock_.start();autoScroll_->start();}
    auto point = canvasPoint(event->position());
    snapGesture_=SnapGrid::load().enabled && !(event->modifiers() & Qt::AltModifier);
    if(snapGesture_) {
        auto grid=SnapGrid::load();
        if(context_==Context::Layout && handle_==0) {
            // Snap one anchor and apply the same delta to every member; never distort a chain.
            for(const auto& placement:document_.state().placements) if(selectedPlacement(placement.id)) {
                point=start_+grid.snap(placement.origin+point-start_)-placement.origin; break;
            }
        } else if((drawing_ && drawingTool_!=Tool::Pencil && drawingTool_!=Tool::Eraser && drawingTool_!=Tool::Fill) ||
                  (context_==Context::Layout && handle_!=5) ||
                  (workingEffect_ && handle_!=6 && handle_!=5 && handle_!=9) ||
                  (maskTarget_ && tool_==MaskTool::Gradient)) point=grid.snap(point);
    }
    guides_.clear();
    if(smartGuides_ && !(event->modifiers() & Qt::AltModifier) && !drawing_ &&
        ((context_==Context::Layout && handle_==0) || (context_==Context::Effects && handle_==6))) {
        const auto ids=selections();QRectF moving;QVector<QRectF> others;
        if(context_==Context::Layout){for(const auto& item:document_.state().placements){const auto bounds=componentBounds(item);if(ids.contains(item.id))moving=moving.united(bounds);else others.append(bounds);}}
        else for(const auto& layer:document_.state().layers)if(!layer.container && layer.visible){const auto bounds=effectBounds(layer);if(ids.contains(layer.id))moving=moving.united(bounds);else others.append(bounds);}
        const auto shifted=moving.translated(point-start_);QPointF correction;
        for(int axis=0;axis<2;++axis) {
            const auto scale=displayScale();double best=6/(zoom_*(axis==0 ? scale.width() : scale.height()));std::optional<QLineF> guide;
            const auto values=[axis](const QRectF& bounds){return axis==0 ? QVector<double>{bounds.left(),bounds.center().x(),bounds.right()} : QVector<double>{bounds.top(),bounds.center().y(),bounds.bottom()};};
            for(const auto& bounds:others)for(const auto target:values(bounds))for(const auto value:values(shifted))if(std::abs(target-value)<best){
                const double delta=target-value;best=std::abs(delta);if(axis==0)correction.setX(delta);else correction.setY(delta);
                guide=axis==0 ? QLineF(target,std::min(bounds.top(),shifted.top())-20,target,std::max(bounds.bottom(),shifted.bottom())+20) : QLineF(std::min(bounds.left(),shifted.left())-20,target,std::max(bounds.right(),shifted.right())+20,target);
            }
            if(guide)guides_.append(*guide);
        }
        point+=correction;if(!guides_.isEmpty())snapGesture_=false;
    }
    if(drawing_) { updateDrawing(point); last_=point; update(); return; }
    if (context_ == Context::Effects && maskTarget_ && tool_ == MaskTool::Brush)
        LightingDocument::paintMask(workingMask_, last_, point, radius_, hardness_, opacity_, gray_);
    last_ = point;
    if(context_==Context::Layout && handle_) {
        const auto a=start_-layoutAnchor_, b=point-layoutAnchor_;
        if(handle_==5) {
            layoutRotation_=(std::atan2(b.y(),b.x())-std::atan2(a.y(),a.x()))*180/3.141592653589793;
            if(snapGesture_) layoutRotation_=SnapGrid{true,15}.snap(layoutRotation_);
        }
        else {
            double minimum=.001, maximum=1000;
            for(const auto& placement:document_.state().placements) if(selectedPlacement(placement.id)) {
                minimum=std::max(minimum,.01/std::min(placement.scale.width(),placement.scale.height()));
                maximum=std::min(maximum,100/std::max(placement.scale.width(),placement.scale.height()));
            }
            layoutScale_=std::clamp(QPointF::dotProduct(a,b)/std::max(.001,QPointF::dotProduct(a,a)),minimum,maximum);
        }
    }
    if(context_==Context::Layout) emit layoutPreviewChanged();
    if (workingEffect_ && handle_!=9) updateEffectGesture(point);
    if (context_ == Context::Effects && maskTarget_ && tool_ == MaskTool::Gradient) {
        for (const auto& layer : document_.state().layers)
            if (layer.id == selected_) workingMask_ = layer.mask;
        LightingDocument::gradientMask(workingMask_, start_, last_);
    }
    update();
}

void LightingCanvas::updateEffectGesture(QPointF point)
{
    if (!originalEffect_) return;
    const auto& original=*originalEffect_; auto changed=original;
    const auto transform=effectTransform(original,seconds_);
    if((handle_==7 || handle_==8) && original.gradientFill) {
        const auto local=transform.inverted().map(point)+QPointF(.5,.5);
        if(handle_==7) changed.gradientStart=local; else changed.gradientEnd=local;
    } else if(handle_==7 || handle_==8) {
        const auto endpoints=gradientEndpoints(original,seconds_); auto a=endpoints.first,b=endpoints.second;
        if(handle_==7) a=point; else b=point;
        const bool centered=centeredGradient(original.effect);
        changed.center=centered ? a : (a+b)/2; changed.extent.setWidth(std::clamp(QLineF(a,b).length()*(centered ? 2 : 1),1.0,10000.0));
        if(centered) changed.extent.setHeight(changed.extent.width());
        changed.rotation=std::remainder(std::atan2(b.y()-a.y(),b.x()-a.x())*180/3.141592653589793-original.spin*seconds_,360.0);
        const double zoom=1+original.zoom/100*std::sin(2*3.141592653589793*seconds_);
        if(std::abs(zoom)>.001){changed.extent.setWidth(changed.extent.width()/zoom);if(centered)changed.extent.setHeight(changed.extent.width());}
    } else if (cropTool_) {
        workingEffect_=original;
        const QSizeF source(original.image.width()*original.crop.width(),original.image.height()*original.crop.height());
        const QSizeF size=source.scaled(original.extent,original.imageFill ? Qt::KeepAspectRatioByExpanding : Qt::KeepAspectRatio);
        const QRectF target(-size.width()/original.extent.width()/2,-size.height()/original.extent.height()/2,
            size.width()/original.extent.width(),size.height()/original.extent.height());
        const auto selected=QRectF(transform.inverted().map(start_),transform.inverted().map(point)).normalized()
            .intersected(QRectF(-0.5,-0.5,1,1)).intersected(target);
        if (selected.width()<0.001 || selected.height()<0.001) return;
        const QRectF relative((selected.x()-target.x())/target.width(),(selected.y()-target.y())/target.height(),
            selected.width()/target.width(),selected.height()/target.height());
        changed.crop={original.crop.x()+relative.x()*original.crop.width(),original.crop.y()+relative.y()*original.crop.height(),
            relative.width()*original.crop.width(),relative.height()*original.crop.height()};
        if (changed.crop.width()<0.001 || changed.crop.height()<0.001) return;
    } else if (handle_==6) {
        changed.center+=point-start_;
        if(snapGesture_) changed.center=SnapGrid::load().snap(changed.center);
        changed.center.setX(std::clamp(changed.center.x(),-10000.0,10000.0));
        changed.center.setY(std::clamp(changed.center.y(),-10000.0,10000.0));
    } else if (handle_==5) {
        const auto center=transform.map(QPointF());
        const auto first=start_-center, last=point-center;
        changed.rotation=std::remainder(original.rotation+(std::atan2(last.y(),last.x())-std::atan2(first.y(),first.x()))*180/3.141592653589793,360.0);
        if(snapGesture_) changed.rotation=SnapGrid{true,15}.snap(changed.rotation);
    } else {
        const auto local=transform.inverted().map(point);
        changed.extent={std::clamp(2*std::abs(local.x())*original.extent.width(),1.0,10000.0),
            std::clamp(2*std::abs(local.y())*original.extent.height(),1.0,10000.0)};
    }
    if (original.maskLinked) changed.mask=transformedMask(original,effectTransform(original).inverted()*effectTransform(changed));
    workingEffect_=changed;
}

void LightingCanvas::mouseReleaseEvent(QMouseEvent* event)
{
    if(previewOnly_) return;
    if(deferredHit_ && event->button()==Qt::LeftButton && QLineF(viewportPoint(start_),event->position()).length()<QApplication::startDragDistance()) {
        const auto point=canvasPoint(event->position());cancelGesture();selectAt(point);return;
    }
    if(marquee_ && event->button()==Qt::LeftButton) {
        last_=canvasPoint(event->position());const QRectF area=QRectF(start_,last_).normalized();auto ids=additiveMarquee_ ? marqueeBefore_ : QSet<QUuid>{};
        if(QLineF(viewportPoint(start_),event->position()).length()>=QApplication::startDragDistance()) {
            if(context_==Context::Layout){for(const auto& item:document_.state().placements)if(editable(item.id) && area.contains(componentBounds(item)))ids.insert(item.id);}
            else for(const auto& layer:document_.state().layers)if(!layer.container && layer.visible && editable(layer.id) && area.contains(effectBounds(layer))) {
                bool visible=true;for(const auto& parent:document_.state().layers)if(parent.id==layer.parentId)visible=parent.visible && parent.transparency<100;
                if(visible && layer.transparency<100)ids.insert(layer.id);
            }
        }
        marquee_=false;selectMany(ids);return;
    }
    if(navigating_ && event->button()==Qt::LeftButton){navigating_=false;update();return;}
    if (event->button() == Qt::MiddleButton || (panning_ && event->button()==Qt::LeftButton)) { panning_ = false; update(); return; }
    if(pendingDrawing_ && event->button()==Qt::LeftButton){pendingDrawing_=false;selectAt(canvasPoint(event->position()));return;}
    if (event->button() != Qt::LeftButton) return;
    // Empty/unsupported drawing gestures still need to release the playback pause started on press.
    if(!dragging_) {if(context_==Context::Effects) emit gestureEnded();return;}
    if(canvasPoint(event->position())!=start_) mouseMoveEvent(event); // Include the final pointer position even without a preceding move event.
    const auto id = selected_;
    const auto created=createdObject_ && workingEffect_ ? workingEffect_->id : QUuid{};
    try {
    if(drawing_) {
        const auto changed=*workingEffect_; const auto mask=workingMask_;
        document_.edit(maskTarget_ ? "Draw mask" : "Draw layer",[&](LightingState& state) {
            if(createdObject_)state.layers.append(changed);
            else for(auto& layer:state.layers) if(layer.id==id) {
                if(maskTarget_) layer.mask=mask; else { layer.image=changed.image; layer.effect=changed.effect; layer.center=changed.center; layer.extent=changed.extent; layer.rotation=changed.rotation; layer.color=changed.color; layer.secondaryColor=changed.secondaryColor; layer.gradientFill=changed.gradientFill; layer.gradientStart=changed.gradientStart; layer.gradientEnd=changed.gradientEnd; layer.gradientColors=changed.gradientColors; layer.gradientPositions=changed.gradientPositions; }
            }
        });
    } else if (context_ == Context::Layout) {
        const auto placements=previewPlacements();
        dragging_=false;
        if(last_!=start_) document_.edit(handle_ ? "Transform components" : "Move components", [&](LightingState& state) {
            state.placements=placements;
        });
    } else if(handle_==9 && workingEffect_) {
        const auto color=workingEffect_->color;
        document_.edit("Fill source color",[&](auto& state) { for(auto& layer:state.layers) if(layer.id==id) { layer.color=color; layer.gradientFill=false; } });
    } else if (workingEffect_ && last_!=start_) {
        const auto ids=selections();QMap<QUuid,EffectLayer> changes;
        for(const auto& layer:document_.state().layers)if(ids.contains(layer.id))changes[layer.id]=transformedSelection(layer);
        document_.edit(cropTool_ ? "Crop image" : "Transform effects",[&](auto& state) {
            for(auto& layer:state.layers)if(changes.contains(layer.id))layer=changes.value(layer.id);
        });
    } else if (maskTarget_) {
        const auto mask = workingMask_;
        for (const auto& layer : document_.state().layers) if (layer.id == id && layer.mask != mask) {
            document_.edit("Edit layer mask", [&](LightingState& state) {
                for (auto& changed : state.layers) if (changed.id == id) changed.mask = mask;
            });
            break;
        }
    }
    } catch(const std::exception& failure) { emit editingError(QString::fromUtf8(failure.what())); }
    cancelGesture();if(!created.isNull() && std::any_of(document_.state().layers.cbegin(),document_.state().layers.cend(),[&](const auto& layer){return layer.id==created;}))select(created); emit gestureEnded();
}

void LightingCanvas::zoomBy(double factor)
{
    if(!std::isfinite(factor) || factor<=0)return; // Reject invalid factors before they poison view/scroll coordinates.
    autoFit_=false; cancelGesture(); zoom_=std::clamp(zoom_*factor,.001,4.0); update();
}

void LightingCanvas::wheelEvent(QWheelEvent* event)
{
    if(previewOnly_) return;
    autoFit_=false;
    cancelGesture();
    const auto before = canvasPoint(event->position());
    zoom_ = std::clamp(zoom_ * std::pow(1.15, event->angleDelta().y() / 120.0), 0.001, 4.0);
    pan_ += event->position() - viewportPoint(before);
    update();
}

void LightingCanvas::keyPressEvent(QKeyEvent* event)
{
    if(event->key()==Qt::Key_Space){spaceHeld_=true;event->accept();return;}
    if (event->key() == Qt::Key_Escape) {
        // Canvas focus consumes Escape; allow returning from fullscreen once no edit needs cancelling.
        if(!dragging_ && !panning_ && !navigating_ && window()->objectName()=="canvasFullscreen") {emit fullscreenRequested();return;}
        cancelGesture(); emit gestureEnded(); return;
    }
    if (!selected_.isNull()) {
        QPointF delta;
        if (event->key() == Qt::Key_Left) delta.setX(-1);
        if (event->key() == Qt::Key_Right) delta.setX(1);
        if (event->key() == Qt::Key_Up) delta.setY(-1);
        if (event->key() == Qt::Key_Down) delta.setY(1);
        if (!delta.isNull()) {
            auto grid=SnapGrid::load(); grid.enabled=grid.enabled && !(event->modifiers() & Qt::AltModifier);
            if (event->modifiers() & Qt::ShiftModifier) delta *= 10;
            if(grid.enabled) {
                delta*=grid.spacing;
                for(const auto& placement:document_.state().placements) if(selectedPlacement(placement.id)) {
                    delta=grid.snap(placement.origin+delta)-placement.origin;break;
                }
            }
            const auto ids=selections();
            document_.edit("Nudge selection", [&](LightingState& state) {
                if(context_==Context::Layout){for(auto& placement:state.placements)if(ids.contains(placement.id))placement.origin+=delta;}
                else for(auto& layer:state.layers)if(ids.contains(layer.id))layer.center+=delta;
            });
            return;
        }
    }
    QWidget::keyPressEvent(event);
}
void LightingCanvas::hideEvent(QHideEvent* event) { cancelGesture(); QWidget::hideEvent(event); }

QRectF LightingCanvas::contentRect() const
{
    if(previewOnly_) return rect();
    return QRectF(16,48,std::max(1,width()-32-(verticalScroll_->isHidden() ? 0 : verticalScroll_->width())),
                  std::max(1,height()-64-(horizontalScroll_->isHidden() ? 0 : horizontalScroll_->height())));
}
void LightingCanvas::setAutoScrollSpeed(double speed)
{
    if(std::isfinite(speed) && speed>=0 && speed<=4000) autoScrollSpeed_=speed;
}
void LightingCanvas::scrollAtEdge()
{
    if(!dragging_ || context_!=Context::Layout) {autoScroll_->stop();return;}
    const double dt=std::clamp(autoScrollClock_.restart()/1000.0,0.0,.05);
    const auto area=contentRect();
    const auto velocity=[](double point,double low,double high) {
        constexpr double band=48;
        if(point<low+band) return -std::clamp((low+band-point)/band,0.0,1.0);
        if(point>high-band) return std::clamp((point-high+band)/band,0.0,1.0);
        return 0.0;
    };
    const QPointF delta(velocity(pointer_.x(),area.left(),area.right()),velocity(pointer_.y(),area.top(),area.bottom()));
    if(delta.isNull() || autoScrollSpeed_==0) return;
    autoFit_=false; pan_-=delta*(autoScrollSpeed_*dt);
    // Re-evaluate the grabbed pointer after camera movement, including when the mouse stands still.
    QMouseEvent move(QEvent::MouseMove,pointer_,mapToGlobal(pointer_.toPoint()),Qt::NoButton,Qt::LeftButton,pointerModifiers_);
    mouseMoveEvent(&move);
}

void LightingCanvas::keyReleaseEvent(QKeyEvent* event)
{
    if(event->key()==Qt::Key_Space){spaceHeld_=false;event->accept();return;}
    QWidget::keyReleaseEvent(event);
}

QVector<QUuid> LightingCanvas::hitsAt(QPointF point) const
{
    QVector<QUuid> hits;
    if(context_==Context::Layout){for(auto it=document_.state().placements.crbegin();it!=document_.state().placements.crend();++it)if(editable(it->id) && (componentBounds(*it).contains(point) || (showLabels_ && componentLabel(*it).contains(point))))hits.append(it->id);return hits;}
    const auto pixel=point.toPoint();
    QVector<const EffectLayer*> order;
    for(const auto& root:document_.state().layers)if(root.parentId.isNull()) {
        order.append(&root);for(const auto& child:document_.state().layers)if(child.parentId==root.id)order.append(&child);
    }
    for(auto position=order.crbegin();position!=order.crend();++position) {
        const auto* it=*position;
        if(!editable(it->id) || it->container || !it->visible || it->transparency==100)continue;
        const EffectLayer* root=nullptr;for(const auto& parent:document_.state().layers)if(parent.id==it->parentId)root=&parent;
        if(root && (!root->visible || root->transparency==100))continue;
        if(!previewChannel_.isNull()) {const auto& target=root ? *root : *it;if(!target.allChannels && !target.channelTargets.contains(previewChannel_))continue;}
        const auto source=renderEffect(*it,seconds_);
        if(source.rect().contains(pixel) && (!root || !root->maskEnabled || renderMask(*root,seconds_).pixelColor(pixel).red()>0) && source.pixelColor(pixel).alpha()>0 && (!it->maskEnabled || renderMask(*it,seconds_).pixelColor(pixel).red()>0))hits.append(it->id);
    }
    return hits;
}
void LightingCanvas::selectAt(QPointF point) {const auto hits=hitsAt(point);selectMany(hits.isEmpty() ? QSet<QUuid>{} : QSet<QUuid>{hits.first()},hits.value(0));}

bool LightingCanvas::editable(const QUuid& id) const
{
    for(const auto& layer:document_.state().layers)if(layer.id==id) {
        if(layer.locked)return false;
        if(layer.parentId.isNull())for(const auto& child:document_.state().layers)if(child.parentId==layer.id && child.locked)return false;
        for(const auto& parent:document_.state().layers)if(parent.id==layer.parentId && parent.locked)return false;
        return true;
    }
    for(const auto& placement:document_.state().placements)if(placement.id==id)return !placement.locked;
    for(const auto& assignment:document_.state().assignments)if(assignment.id==id)
        return !assignment.placements.isEmpty() && std::none_of(document_.state().placements.cbegin(),document_.state().placements.cend(),[&](const auto& item){return assignment.placements.contains(item.id) && item.locked;});
    return false;
}
void LightingCanvas::selectMany(const QSet<QUuid>& ids,const QUuid& primary)
{
    cancelGesture();selectedIds_=ids;
    selected_=ids.contains(primary) ? primary : ids.isEmpty() ? QUuid{} : *ids.begin();
    emit selectionChanged();update();
}
QSet<QUuid> LightingCanvas::selections() const
{
    QSet<QUuid> ids;
    if(context_==Context::Layout) {
        for(const auto& item:document_.state().placements)if(selectedPlacement(item.id))ids.insert(item.id);
    } else {
        for(const auto& item:document_.state().layers)if(editable(item.id) &&
            (selectedIds_.contains(item.id) || (selectedIds_.contains(item.parentId) && editable(item.parentId))) && !item.container)ids.insert(item.id);
    }
    return ids;
}
QRectF LightingCanvas::effectBounds(const EffectLayer& layer) const
{
    return layer.effect==EffectKind::StaticColor ? QRectF(0,0,640,400) : effectTransform(layer,seconds_).mapRect(QRectF(-.5,-.5,1,1));
}
EffectLayer LightingCanvas::transformedSelection(const EffectLayer& layer) const
{
    if(!workingEffect_ || !originalEffect_ || !selections().contains(layer.id))return layer;
    if(layer.id==originalEffect_->id)return *workingEffect_;
    if(cropTool_ || handle_==7 || handle_==8 || drawing_)return layer;
    auto changed=layer;const auto& from=*originalEffect_;const auto& to=*workingEffect_;
    QTransform transform;transform.translate(to.center.x(),to.center.y());transform.rotate(to.rotation-from.rotation);
    transform.scale(to.extent.width()/from.extent.width(),to.extent.height()/from.extent.height());transform.translate(-from.center.x(),-from.center.y());
    const auto center=effectTransform(layer,seconds_).map(QPointF());changed.center+=transform.map(center)-center;changed.rotation=std::remainder(layer.rotation+to.rotation-from.rotation,360.0);
    changed.extent={layer.extent.width()*to.extent.width()/from.extent.width(),layer.extent.height()*to.extent.height()/from.extent.height()};
    if(layer.maskLinked)changed.mask=transformedMask(layer,effectTransform(layer).inverted()*effectTransform(changed));
    return changed;
}
void LightingCanvas::alignSelection(const QString& alignment)
{
    try {
    cancelGesture();const auto ids=selections();QVector<QPair<QUuid,QRectF>> items;QMap<QUuid,QSet<QUuid>> members;
    const auto append=[&](const QUuid& id,const QSet<QUuid>& group){
        QRectF bounds;
        if(context_==Context::Layout){for(const auto& item:document_.state().placements)if(group.contains(item.id))bounds=bounds.united(componentBounds(item));}
        else for(const auto& item:document_.state().layers)if(group.contains(item.id))bounds=bounds.united(effectBounds(item));
        if(!bounds.isEmpty()){items.append({id,bounds});members[id]=group;}
    };
    QSet<QUuid> grouped;
    if(context_==Context::Layout) {
        for(const auto& assignment:document_.state().assignments)if(selectedIds_.contains(assignment.id)) {
            QSet<QUuid> group;for(const auto& id:assignment.placements)if(ids.contains(id))group.insert(id);
            append(assignment.id,group);grouped.unite(group);
        }
        for(const auto& item:document_.state().placements)if(ids.contains(item.id) && !grouped.contains(item.id))append(item.id,{item.id});
    } else {
        for(const auto& root:document_.state().layers)if(root.parentId.isNull() && selectedIds_.contains(root.id)) {
            QSet<QUuid> group;for(const auto& item:document_.state().layers)if(ids.contains(item.id) && (item.id==root.id || item.parentId==root.id))group.insert(item.id);
            append(root.id,group);grouped.unite(group);
        }
        for(const auto& item:document_.state().layers)if(ids.contains(item.id) && !grouped.contains(item.id))append(item.id,{item.id});
    }
    const bool distribute=alignment.startsWith("distribute");
    if(items.size()<(distribute ? 3 : 2)){emit editingError(distribute ? "Select at least three objects to distribute." : "Select at least two objects to align.");return;}
    const bool horizontal=alignment=="left" || alignment=="center" || alignment=="right" || alignment=="distribute-horizontal";
    QRectF bounds;for(const auto& item:items)bounds=bounds.united(item.second);
    QMap<QUuid,QPointF> deltas;
    if(distribute) {
        std::stable_sort(items.begin(),items.end(),[horizontal](const auto& a,const auto& b){return horizontal ? a.second.center().x()<b.second.center().x() : a.second.center().y()<b.second.center().y();});
        const double first=horizontal ? items.first().second.center().x() : items.first().second.center().y();
        const double last=horizontal ? items.last().second.center().x() : items.last().second.center().y();
        for(int i=0;i<items.size();++i){const auto& item=items[i];const double delta=first+(last-first)*i/(items.size()-1)-(horizontal ? item.second.center().x() : item.second.center().y());deltas[item.first]=horizontal ? QPointF(delta,0) : QPointF(0,delta);}
    } else for(const auto& item:items) {
        const auto rect=item.second;
        deltas[item.first]=alignment=="left" ? QPointF(bounds.left()-rect.left(),0) : alignment=="right" ? QPointF(bounds.right()-rect.right(),0) : alignment=="center" ? QPointF(bounds.center().x()-rect.center().x(),0) : alignment=="top" ? QPointF(0,bounds.top()-rect.top()) : alignment=="bottom" ? QPointF(0,bounds.bottom()-rect.bottom()) : QPointF(0,bounds.center().y()-rect.center().y());
    }
    QMap<QUuid,QPointF> memberDeltas;for(auto it=deltas.cbegin();it!=deltas.cend();++it)for(const auto& id:members[it.key()])memberDeltas[id]=it.value();deltas=memberDeltas;
    document_.edit("Align selection",[&](auto& state){
        if(context_==Context::Layout){for(auto& item:state.placements)if(deltas.contains(item.id))item.origin+=deltas[item.id];}
        else for(auto& item:state.layers)if(deltas.contains(item.id)){auto before=item;item.center+=deltas[item.id];if(item.maskLinked)item.mask=transformedMask(before,effectTransform(before).inverted()*effectTransform(item));}
    });

    } catch(const std::exception& error) {emit editingError(QString::fromUtf8(error.what()));}
}
void LightingCanvas::lockSelection(bool locked)
{
    try {
    auto ids=selectedIds_;
    if(context_==Context::Layout)for(const auto& assignment:document_.state().assignments)if(selectedIds_.contains(assignment.id))for(const auto& id:assignment.placements)ids.insert(id);
    document_.edit(locked ? "Lock selection" : "Unlock selection",[&](auto& state){
        if(context_==Context::Layout){for(auto& item:state.placements)if(ids.contains(item.id))item.locked=locked;}
        else for(auto& item:state.layers)if(ids.contains(item.id))item.locked=locked;
    });

    } catch(const std::exception& error) {emit editingError(QString::fromUtf8(error.what()));}
}
void LightingCanvas::deleteSelection()
{
    try {
    const auto ids=selections();
    if(ids.isEmpty() && !selectedIds_.isEmpty() && std::none_of(document_.state().layers.cbegin(),document_.state().layers.cend(),[&](const auto& item){return selectedIds_.contains(item.id) && item.container && !item.locked;})){emit editingError("Selection is locked or unavailable.");return;}
    for(const auto& layer:document_.state().layers)if(selectedIds_.contains(layer.parentId) && layer.locked){emit editingError("Unlock the layer's objects before deleting their parent.");return;}
    if(ids.isEmpty() && selectedIds_.isEmpty())return;
    // Remove routing references atomically with placements; never leave dangling LED ownership.
    document_.edit("Delete selection",[&](auto& state){
        if(context_==Context::Layout){
            state.placements.removeIf([&](const auto& item){return ids.contains(item.id);});
            for(auto& assignment:state.assignments)assignment.placements.removeIf([&](const auto& id){return ids.contains(id);});
            QSet<QUuid> removed;for(const auto& assignment:state.assignments)if(assignment.software && assignment.placements.isEmpty())removed.insert(assignment.id);
            state.assignments.removeIf([&](const auto& item){return removed.contains(item.id);});for(auto& layer:state.layers)layer.channelTargets.removeIf([&](const auto& id){return removed.contains(id);});
        } else state.layers.removeIf([&](const auto& item){return ids.contains(item.id) || (selectedIds_.contains(item.id) && item.container && !item.locked);});
    });selectMany({});

    } catch(const std::exception& error) {emit editingError(QString::fromUtf8(error.what()));}
}
void LightingCanvas::duplicateSelection()
{
    try {
    auto ids=selections();QSet<QUuid> copies;
    if(context_==Context::Layout) {
        if(document_.state().placements.size()+ids.size()>256){emit editingError("Duplicate exceeds the 256 component limit.");return;}
        document_.edit("Duplicate components",[&](auto& state){const auto before=state.placements;for(auto item:before)if(ids.contains(item.id)){item.id=QUuid::createUuid();item.origin+=QPointF(10,10);copies.insert(item.id);state.placements.append(item);}});
    } else {
        for(const auto& item:document_.state().layers)if(selectedIds_.contains(item.id) && item.container && !item.locked)ids.insert(item.id);
        // Copying a whole layer includes locked children; reading their content does not edit the originals.
        for(const auto& item:document_.state().layers)if(selectedIds_.contains(item.parentId) && ids.contains(item.parentId))ids.insert(item.id);
        if(ids.isEmpty()){emit editingError("Select an unlocked object or layer to duplicate.");return;}
        if(document_.state().layers.size()+ids.size()>64){emit editingError("Duplicate exceeds the 64 layer/object limit.");return;}
        QMap<QUuid,QUuid> mapping;for(const auto& id:ids)mapping[id]=QUuid::createUuid();
        document_.edit("Duplicate objects",[&](auto& state){const auto before=state.layers;for(auto item:before)if(ids.contains(item.id)){item.id=mapping[item.id];if(mapping.contains(item.parentId))item.parentId=mapping[item.parentId];item.name+=" copy";const auto beforeMove=item;item.center+=QPointF(10,10);if(item.maskLinked)item.mask=transformedMask(beforeMove,effectTransform(beforeMove).inverted()*effectTransform(item));copies.insert(item.id);state.layers.append(item);}});
    }
    selectMany(copies);

    } catch(const std::exception& error) {emit editingError(QString::fromUtf8(error.what()));}
}
