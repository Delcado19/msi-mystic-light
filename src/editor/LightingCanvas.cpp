#include "LightingCanvas.h"
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

namespace {
bool centeredGradient(EffectKind kind) { return kind==EffectKind::RadialGradient || kind==EffectKind::ConicalGradient; }
QPair<QPointF,QPointF> gradientEndpoints(const EffectLayer& layer) {
    const auto transform=effectTransform(layer);
    if(layer.gradientFill) return {transform.map(layer.gradientStart-QPointF(.5,.5)),transform.map(layer.gradientEnd-QPointF(.5,.5))};
    return {transform.map(QPointF(centeredGradient(layer.effect) ? 0 : -.5,0)),transform.map(QPointF(.5,0))};
}
}

LightingCanvas::LightingCanvas(LightingDocument& document, Context context, QWidget* parent)
    : QWidget(parent), document_(document), context_(context)
{
    setMinimumSize(360, 280);
    emptyAction_=new QPushButton(context==Context::Layout ? "Add a component" : "Add an effect layer",this);
    emptyAction_->setObjectName("canvasAdd"); emptyAction_->setProperty("primary",true); emptyAction_->setFixedSize(190,38);
    connect(emptyAction_,&QPushButton::clicked,this,&LightingCanvas::addRequested);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setAccessibleName(context == Context::Layout ? "Component layout canvas" : "Effect and layer mask canvas");
    connect(&document_, &LightingDocument::changed, this, [this] {
        cancelGesture();
        if(previewOnly_) fitToView();
        rendered_ = LightingDocument::render(document_.state(),seconds_,previewChannel_);
        emptyAction_->setVisible(!previewOnly_ && (context_==Context::Effects ? document_.state().layers.isEmpty() : document_.state().placements.isEmpty()));
        update();
    });
    rendered_ = LightingDocument::render(document_.state(),seconds_,previewChannel_);
}

void LightingCanvas::fitToView()
{
    cancelGesture(); autoFit_=true; pan_={};
    if(previewOnly_) {
        previewBounds_=QRectF(0,0,640,400);
        zoom_=std::clamp(std::min((width()-60)/previewBounds_.width(),(height()-48)/previewBounds_.height()),.1,2.0);
    } else zoom_=std::clamp(std::min((width()-80)/640.0,(height()-100)/400.0),0.1,4.0);
    update();
}
void LightingCanvas::resizeEvent(QResizeEvent*)
{
    if(autoFit_) fitToView();
    if(previewOnly_) emptyAction_->hide();
    emptyAction_->move((width()-emptyAction_->width())/2,height()/2+32);
}

void LightingCanvas::setPreviewFrame(const QImage& frame, double seconds, const QUuid& channel)
{
    seconds_ = seconds; previewChannel_=channel; rendered_ = frame; update();
}

QPointF LightingCanvas::offset() const
{
    if(previewOnly_) return QPointF(width()/2.0,height()/2.0+18)-previewBounds_.center()*zoom_;
    return QPointF((width() - 640 * zoom_) / 2, (height() - 400 * zoom_) / 2) + pan_;
}
QPointF LightingCanvas::canvasPoint(QPointF viewport) const { return (viewport - offset()) / zoom_; }
QPointF LightingCanvas::viewportPoint(QPointF canvas) const { return offset()+canvas*zoom_; }
void LightingCanvas::select(const QUuid& id)
{
    cancelGesture();
    selected_ = id;
    emit selectionChanged();
    update();
}
void LightingCanvas::setMaskTarget(bool enabled) { cancelGesture(); maskTarget_ = enabled; setCursor(enabled ? Qt::CrossCursor : Qt::ArrowCursor); update(); }
void LightingCanvas::setBrush(double radius, double hardness, double opacity, int gray)
{
    radius_ = radius; hardness_ = hardness; opacity_ = opacity; gray_ = gray; update();
}
void LightingCanvas::cancelGesture()
{
    drawing_=false; drawingBase_={}; fillReference_={};
    dragging_ = false;
    panning_ = false;
    workingMask_ = {}; originalEffect_.reset(); workingEffect_.reset(); handle_ = 0; layoutScale_=1; layoutRotation_=0;
    update();
}

QVector<QPointF> LightingCanvas::componentPoints(const ComponentPlacement& placement) const
{
    return document_.componentPoints(placement);
}

void LightingCanvas::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.fillRect(rect(), QColor("#17191f"));
    painter.setPen(QColor("#a6afc2"));
    painter.drawText(QRect(20,12,width()-40,24),Qt::AlignLeft|Qt::AlignVCenter,
        QString("%1    /    640 x 400    /    %2%").arg(previewOnly_ ? QString("LIVE HARDWARE / %1 s").arg(seconds_,0,'f',2) : context_==Context::Layout ? "LAYOUT" : "EFFECT CANVAS").arg(qRound(zoom_*100)));
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
    painter.translate(offset());
    painter.scale(zoom_, zoom_);
    painter.setPen(QPen(QColor("#626d85"),1/zoom_)); painter.setBrush(Qt::NoBrush);
    if(!previewOnly_) {
    painter.drawRect(QRectF(QPointF(),LightingDocument::canvasSize()).adjusted(-1,-1,1,1));
    painter.setClipRect(QRect(QPoint(), LightingDocument::canvasSize()));
    for (int y = 0; y < 400; y += 16)
        for (int x = 0; x < 640; x += 16)
            painter.fillRect(x, y, 16, 16, (x / 16 + y / 16) % 2 ? QColor("#484b50") : QColor("#35383d"));
    }
    auto frame = rendered_;
    if (context_ == Context::Effects) {
        if (workingEffect_) {
            auto state=document_.state();
            for (auto& layer : state.layers) if (layer.id==selected_) layer=*workingEffect_;
            frame=LightingDocument::render(state,seconds_,previewChannel_);
        }
        if (maskTarget_) {
            auto state=document_.state();
            for (auto& layer : state.layers) if (layer.id==selected_) {
                if (!workingMask_.isNull()) layer.mask=workingMask_;
                layer.warp=0; layer.maskLinked=false;
            }
            frame=LightingDocument::render(state,0,previewChannel_); // Paint the base mask, without animated spatial distortion.
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
        painter.drawImage(0, 0, frame);
        if(maskTarget_ && tool_==MaskTool::Brush && underMouse()) {
            const auto cursor=canvasPoint(mapFromGlobal(QCursor::pos()));
            painter.setPen(QPen(QColor("#172337"),3/zoom_)); painter.setBrush(Qt::NoBrush); painter.drawEllipse(cursor,radius_,radius_);
            painter.setPen(QPen(QColor("#ebf2ff"),1/zoom_)); painter.drawEllipse(cursor,radius_,radius_);
        }
        if (dragging_ && maskTarget_ && tool_ == MaskTool::Gradient) {
            painter.setPen(QPen(QColor("#a9c2ff"), 1 / zoom_));
            painter.drawLine(start_, last_);
        }
        if(!maskTarget_ && drawingTool_==Tool::Gradient) for(const auto& stored:document_.state().layers) if(stored.id==selected_ && (isGradient(stored.effect) || stored.gradientFill)) {
            const auto& layer=workingEffect_ ? *workingEffect_ : stored;
            const auto endpoints=gradientEndpoints(layer); const auto a=endpoints.first,b=endpoints.second;
            painter.setPen(QPen(QColor("#101722"),4/zoom_)); painter.drawLine(a,b);
            painter.setPen(QPen(QColor("#e8f0ff"),2/zoom_)); painter.drawLine(a,b);
            const auto direction=(b-a)/std::max(1.0,QLineF(a,b).length()); const QPointF side(-direction.y(),direction.x());
            painter.drawLine(b,b-direction*(13/zoom_)+side*(6/zoom_)); painter.drawLine(b,b-direction*(13/zoom_)-side*(6/zoom_));
            painter.setBrush(layer.color); painter.drawEllipse(a,6/zoom_,6/zoom_);
            painter.setBrush(layer.secondaryColor); painter.drawEllipse(b,6/zoom_,6/zoom_);
        }
        if (!maskTarget_ && drawingTool_==Tool::Move) for (const auto& stored : document_.state().layers) if (stored.id==selected_ && stored.effect!=EffectKind::StaticColor) {
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
        return;
    }
    // The same live raster explains sampling positions; Layout edits placements only.
    painter.drawImage(0,0,frame);
    painter.setClipping(false);
    painter.setRenderHint(QPainter::Antialiasing);
    auto sampled=document_.sampleComponents(frame);
    QMap<QUuid,QImage> placementFrames; QSet<QUuid> nativePlacements;
    bool scoped=false; for(const auto& layer:document_.state().layers) if(!layer.allChannels) scoped=true;
    for(const auto& assignment:document_.state().assignments) {
        auto channelFrame=frame; auto channelColors=sampled;
        if(assignment.software && (scoped || assignment.timeScale!=1 || assignment.timeOffset!=0)) { channelFrame=document_.renderForChannel(assignment.id,seconds_); channelColors=document_.sampleComponents(channelFrame); }
        for(const auto& id:assignment.placements) {
            if(assignment.software) { sampled[id]=channelColors[id]; placementFrames[id]=channelFrame; }
            else {
                nativePlacements.insert(id); QColor color=Qt::black;
                if(assignment.native.id!="off" && !assignment.native.colors.isEmpty()) { const auto rgb=assignment.native.colors.first(); color=QColor(rgb.r,rgb.g,rgb.b); }
                if(sampled.contains(id)) sampled[id].fill(color);
            }
        }
    }
    if(!previewOnly_) for(const auto& assignment:document_.state().assignments) {
        QRectF bounds;
        for(auto placement:document_.state().placements) if(assignment.placements.contains(placement.id))
            for(const auto& point:componentPoints(previewPlacement(placement))) bounds=bounds.united(QRectF(point,QSizeF(.01,.01)));
        if(bounds.isEmpty()) continue;
        bounds=bounds.adjusted(-18,-25,18,18);
        painter.setBrush(Qt::NoBrush); painter.setPen(QPen(QColor("#8196b8"),1/zoom_,Qt::DashLine));
        painter.drawRoundedRect(bounds,8,8);
        const QRectF title(bounds.left(),bounds.top()-painter.fontMetrics().height()-6,painter.fontMetrics().horizontalAdvance(assignment.channelId)+12,painter.fontMetrics().height()+4);
        painter.fillRect(title,QColor("#202735")); painter.setPen(QColor("#dce7ff")); painter.drawText(title,Qt::AlignCenter,assignment.channelId);
    }
    for (auto placement : document_.state().placements) {
        placement=previewPlacement(placement);
        const auto points = componentPoints(placement);
        const auto& definition = document_.catalog()[placement.componentId];
        QRectF body;
        for(const auto& point:points) body=body.united(QRectF(point,QSizeF(.01,.01)));
        body=body.adjusted(-10,-10,10,10);
        painter.setPen(QPen(selectedPlacement(placement.id) ? QColor("#a9c2ff") : QColor("#657084"),1/zoom_)); painter.setBrush(QColor("#1b2028"));
        painter.drawRoundedRect(body,7,7);
        const bool fan=(definition.id+definition.name).contains("fan",Qt::CaseInsensitive) || definition.name.contains("M25");
        if(fan) { painter.drawEllipse(body.adjusted(4,4,-4,-4)); painter.setBrush(QColor("#323e50")); painter.drawEllipse(body.center(),6,6); }
        painter.setPen(QPen(selectedPlacement(placement.id) ? QColor("#a9c2ff") : Qt::white, 1.5 / zoom_));
        const bool keyboard=definition.name.contains("Vulcan",Qt::CaseInsensitive) || definition.name.contains("Apex",Qt::CaseInsensitive);
        QSet<QPair<int,int>> drawnKeys;
        for (int i = 0; i < points.size(); ++i) {
            const auto point = points[i];
            if(keyboard) { const auto key=qMakePair(qRound(point.x()*10),qRound(point.y()*10)); if(drawnKeys.contains(key)) continue; drawnKeys.insert(key); }
            if (!QRectF(frame.rect()).adjusted(-20, -20, 20, 20).contains(point)) continue;
            const auto samplingFrame=placementFrames.value(placement.id,frame);
            const QColor light = dragging_ && selectedPlacement(placement.id) && !nativePlacements.contains(placement.id) ?
                (QRectF(samplingFrame.rect()).contains(point) ? samplingFrame.pixelColor(point.toPoint()) : QColor(Qt::black)) : sampled[placement.id].value(definition.mapping[i],Qt::black);
            painter.setBrush(light);
            if(keyboard) painter.drawRoundedRect(QRectF(point-QPointF(4,4),QSizeF(8,8)),1.5,1.5); else painter.drawEllipse(point, 6, 6);

        }
        if (!previewOnly_ && placement.id==selected_ && QRectF(frame.rect()).contains(placement.origin))
            painter.drawText(body.topLeft() + QPointF(0, -6), painter.fontMetrics().elidedText(definition.name,Qt::ElideRight,110));
    }
    if(!previewOnly_ && !selected_.isNull()) {
        auto bounds=selectionBounds();

        painter.setPen(QPen(QColor("#a9c2ff"),1.5/zoom_)); painter.setBrush(Qt::NoBrush);
        painter.drawRect(bounds);
        for(const auto point:{bounds.topLeft(),bounds.topRight(),bounds.bottomLeft(),bounds.bottomRight()}) {
            painter.setBrush(QColor("#202838")); painter.drawRect(QRectF(point-QPointF(5/zoom_,5/zoom_),QSizeF(10/zoom_,10/zoom_)));
        }
        const QPointF top(bounds.center().x(),bounds.top()), rotation=top-QPointF(0,24/zoom_);
        painter.drawLine(top,rotation); painter.drawEllipse(rotation,6/zoom_,6/zoom_);
    }
}

QRectF LightingCanvas::selectionBounds() const
{
    QRectF bounds;
    for(const auto& placement:document_.state().placements) if(selectedPlacement(placement.id))
        for(const auto& point:componentPoints(previewPlacement(placement))) bounds=bounds.united(QRectF(point,QSizeF(.01,.01)));
    return bounds.isEmpty() ? QRectF() : bounds.adjusted(-15,-15,15,15);
}
ComponentPlacement LightingCanvas::previewPlacement(ComponentPlacement placement) const
{
    if(!dragging_ || !selectedPlacement(placement.id)) return placement;
    if(handle_==0) placement.origin+=last_-start_;
    else {
        QTransform transform; transform.translate(layoutBounds_.center().x(),layoutBounds_.center().y());
        transform.rotate(layoutRotation_); transform.scale(layoutScale_,layoutScale_);
        transform.translate(-layoutBounds_.center().x(),-layoutBounds_.center().y());
        placement.origin=transform.map(placement.origin);
        placement.scale*=layoutScale_;
        placement.rotation=std::remainder(placement.rotation+layoutRotation_,360.0);
    }
    return placement;
}

QPointF LightingCanvas::paintPoint(QPointF point) const
{
    if(maskTarget_) return point;
    const auto local=effectTransform(*originalEffect_).inverted().map(point)+QPointF(.5,.5);
    return {local.x()*640,local.y()*400};
}

void LightingCanvas::beginDrawing(QPointF point)
{
    EffectLayer layer; bool found=false;
    for(const auto& candidate:document_.state().layers) if(candidate.id==selected_) { layer=candidate; found=true; }
    if(!found && drawingTool_==Tool::Eraser) return;
    if(found && !maskTarget_ && drawingTool_==Tool::Eraser && layer.effect!=EffectKind::Paint) {
        // Procedural sources stay editable: erasing edits their existing mask instead of flattening pixels.
        emit maskEditingRequested(); maskTarget_=true;
    }
    if(maskTarget_ && !found) return;
    const bool gradient=drawingTool_==Tool::Gradient;
    const bool solid=layer.effect==EffectKind::StaticColor || (layer.effect>=EffectKind::Circle && layer.effect<=EffectKind::Moon);
    if(found && !maskTarget_ && drawingTool_==Tool::Fill && solid && renderEffect(layer).pixelColor(point.toPoint()).alpha()>0) {
        originalEffect_=workingEffect_=layer; workingEffect_->color=drawColor_; workingEffect_->gradientFill=false; dragging_=true; handle_=9; return;
    }
    // Bucket fills use the selected procedural source as their boundary, while keeping that source editable below the paint result.
    const auto reference=!maskTarget_ && found && drawingTool_==Tool::Fill && layer.effect!=EffectKind::Paint ? renderEffect(layer) : QImage{};
    if(!maskTarget_ && (!found || (gradient ? !isGradient(layer.effect) && layer.effect!=EffectKind::Paint && !solid : layer.effect!=EffectKind::Paint))) {
        if(document_.state().layers.size()>=64) { emit editingError("Layer limit reached. Select an existing paint layer or remove a layer first."); return; }
        select(gradient ? document_.addLayer(EffectKind::LinearGradient) : document_.addPaintLayer());
        for(const auto& candidate:document_.state().layers) if(candidate.id==selected_) layer=candidate;
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
    if(gradient && !maskTarget_ && originalEffect_->effect>=EffectKind::Circle && originalEffect_->effect<=EffectKind::Moon) {
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

bool LightingCanvas::selectedPlacement(const QUuid& id) const
{
    if(id==selected_) return true;
    for(const auto& assignment:document_.state().assignments)
        if(assignment.id==selected_) return assignment.placements.contains(id);
    return false;
}

void LightingCanvas::mousePressEvent(QMouseEvent* event)
{
    if(previewOnly_) return;
    setFocus();
    if (event->button() == Qt::MiddleButton) { cancelGesture(); autoFit_=false; panning_ = true; last_ = event->position(); return; }
    if (event->button() != Qt::LeftButton) return;
    start_ = last_ = canvasPoint(event->position());
    if (context_==Context::Effects && !QRectF(QPointF(), LightingDocument::canvasSize()).contains(start_)) return;
    if(context_==Context::Effects && drawingTool_!=Tool::Move) {
        emit effectGestureStarted();
        if(drawingTool_==Tool::Gradient && !maskTarget_) for(const auto& layer:document_.state().layers) if(layer.id==selected_ && (isGradient(layer.effect) || layer.gradientFill)) {
            const auto endpoints=gradientEndpoints(layer); const auto a=endpoints.first,b=endpoints.second;
            if(QLineF(start_,a).length()<12/zoom_ || QLineF(start_,b).length()<12/zoom_) {
                handle_=QLineF(start_,a).length()<12/zoom_ ? 7 : 8;
                originalEffect_=workingEffect_=layer; dragging_=true; return;
            }
        }
        beginDrawing(start_); return;
    }
    if (context_ == Context::Layout) {
        layoutBounds_=selectionBounds();
        if(!layoutBounds_.isEmpty()) {
            for(const auto corner:{layoutBounds_.topLeft(),layoutBounds_.topRight(),layoutBounds_.bottomLeft(),layoutBounds_.bottomRight()})
                if(QLineF(start_,corner).length()<12/zoom_) handle_=1;
            if(QLineF(start_,QPointF(layoutBounds_.center().x(),layoutBounds_.top()-24/zoom_)).length()<12/zoom_) handle_=5;
            if(handle_) { dragging_=true; return; }
        }
        QUuid hit;
        for (auto it = document_.state().placements.crbegin(); it != document_.state().placements.crend(); ++it) {
            QRectF body;
            for(const auto point:componentPoints(*it)) body=body.united(QRectF(point,QSizeF(.01,.01)));
            if(body.adjusted(-10,-10,10,10).contains(start_)) { hit=it->id; origin_=it->origin; break; }
            for (auto point : componentPoints(*it))
                if (QLineF(point, start_).length() < 12 / zoom_) { hit = it->id; origin_ = it->origin; break; }
            if (!hit.isNull()) break;
        }
        if(hit.isNull() && !selected_.isNull() && layoutBounds_.contains(start_)) { dragging_=true; return; }
        // A header selected in the tree keeps its chain together when dragging any member.
        if(hit.isNull() || !selectedPlacement(hit)) select(hit);
        dragging_ = !hit.isNull();
    } else if (!maskTarget_) {
        emit effectGestureStarted();
        for (const auto& layer : document_.state().layers) if (layer.id==selected_ && layer.effect!=EffectKind::StaticColor) {
            const auto transform=effectTransform(layer);
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
    if (panning_) { pan_ += event->position() - last_; last_ = event->position(); update(); return; }
    if (!dragging_) { if(maskTarget_) update(); return; }
    const auto point = canvasPoint(event->position());
    if(drawing_) { updateDrawing(point); last_=point; update(); return; }
    if (context_ == Context::Effects && maskTarget_ && tool_ == MaskTool::Brush)
        LightingDocument::paintMask(workingMask_, last_, point, radius_, hardness_, opacity_, gray_);
    last_ = point;
    if(context_==Context::Layout && handle_) {
        const auto a=start_-layoutBounds_.center(), b=point-layoutBounds_.center();
        if(handle_==5) layoutRotation_=(std::atan2(b.y(),b.x())-std::atan2(a.y(),a.x()))*180/3.141592653589793;
        else {
            double minimum=.001, maximum=1000;
            for(const auto& placement:document_.state().placements) if(selectedPlacement(placement.id)) {
                minimum=std::max(minimum,.01/std::min(placement.scale.width(),placement.scale.height()));
                maximum=std::min(maximum,100/std::max(placement.scale.width(),placement.scale.height()));
            }
            layoutScale_=std::clamp(QLineF(QPointF(),b).length()/std::max(.001,QLineF(QPointF(),a).length()),minimum,maximum);
        }
    }
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
    const auto transform=effectTransform(original);
    if((handle_==7 || handle_==8) && original.gradientFill) {
        const auto local=transform.inverted().map(point)+QPointF(.5,.5);
        if(handle_==7) changed.gradientStart=local; else changed.gradientEnd=local;
    } else if(handle_==7 || handle_==8) {
        const auto endpoints=gradientEndpoints(original); auto a=endpoints.first,b=endpoints.second;
        if(handle_==7) a=point; else b=point;
        const bool centered=centeredGradient(original.effect);
        changed.center=centered ? a : (a+b)/2; changed.extent.setWidth(std::clamp(QLineF(a,b).length()*(centered ? 2 : 1),1.0,10000.0));
        if(centered) changed.extent.setHeight(changed.extent.width());
        changed.rotation=std::atan2(b.y()-a.y(),b.x()-a.x())*180/3.141592653589793;
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
        changed.center.setX(std::clamp(changed.center.x(),-10000.0,10000.0));
        changed.center.setY(std::clamp(changed.center.y(),-10000.0,10000.0));
    } else if (handle_==5) {
        const auto first=start_-original.center, last=point-original.center;
        changed.rotation=std::remainder(original.rotation+(std::atan2(last.y(),last.x())-std::atan2(first.y(),first.x()))*180/3.141592653589793,360.0);
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
    if (event->button() == Qt::MiddleButton) { panning_ = false; return; }
    if (event->button() != Qt::LeftButton || !dragging_) return;
    const auto id = selected_;
    try {
    if(drawing_) {
        const auto changed=*workingEffect_; const auto mask=workingMask_;
        document_.edit(maskTarget_ ? "Draw mask" : "Draw layer",[&](LightingState& state) {
            for(auto& layer:state.layers) if(layer.id==id) {
                if(maskTarget_) layer.mask=mask; else { layer.image=changed.image; layer.effect=changed.effect; layer.center=changed.center; layer.extent=changed.extent; layer.rotation=changed.rotation; layer.color=changed.color; layer.secondaryColor=changed.secondaryColor; layer.gradientFill=changed.gradientFill; layer.gradientStart=changed.gradientStart; layer.gradientEnd=changed.gradientEnd; layer.gradientColors=changed.gradientColors; layer.gradientPositions=changed.gradientPositions; }
            }
        });
    } else if (context_ == Context::Layout) {
        if (last_ != start_) document_.edit(handle_ ? "Transform components" : "Move components", [&](LightingState& state) {
            for (auto& placement : state.placements) placement=previewPlacement(placement);
        });
    } else if(handle_==9 && workingEffect_) {
        const auto color=workingEffect_->color;
        document_.edit("Fill source color",[&](auto& state) { for(auto& layer:state.layers) if(layer.id==id) { layer.color=color; layer.gradientFill=false; } });
    } else if (workingEffect_ && last_!=start_) {
        const auto effect=*workingEffect_;
        document_.edit(cropTool_ ? "Crop image" : "Transform effect",[&](auto& state) {
            for (auto& layer : state.layers) if (layer.id==id) {
                layer.center=effect.center; layer.extent=effect.extent; layer.rotation=effect.rotation; layer.crop=effect.crop; layer.gradientStart=effect.gradientStart; layer.gradientEnd=effect.gradientEnd;
            }
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
    cancelGesture(); emit gestureEnded();
}

void LightingCanvas::zoomBy(double factor)
{
    autoFit_=false; cancelGesture(); zoom_=std::clamp(zoom_*factor,.1,4.0); update();
}

void LightingCanvas::wheelEvent(QWheelEvent* event)
{
    if(previewOnly_) return;
    autoFit_=false;
    cancelGesture();
    const auto before = canvasPoint(event->position());
    zoom_ = std::clamp(zoom_ * std::pow(1.15, event->angleDelta().y() / 120.0), 0.25, 4.0);
    pan_ += event->position() - (before * zoom_ + offset());
    update();
}

void LightingCanvas::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) { cancelGesture(); emit gestureEnded(); return; }
    if (context_ == Context::Layout && !selected_.isNull()) {
        QPointF delta;
        if (event->key() == Qt::Key_Left) delta.setX(-1);
        if (event->key() == Qt::Key_Right) delta.setX(1);
        if (event->key() == Qt::Key_Up) delta.setY(-1);
        if (event->key() == Qt::Key_Down) delta.setY(1);
        if (!delta.isNull()) {
            if (event->modifiers() & Qt::ShiftModifier) delta *= 10;
            document_.edit("Nudge component", [&](LightingState& state) {
                for (auto& placement : state.placements) if (selectedPlacement(placement.id)) placement.origin += delta;
            });
            return;
        }
    }
    QWidget::keyPressEvent(event);
}
void LightingCanvas::hideEvent(QHideEvent* event) { cancelGesture(); QWidget::hideEvent(event); }
