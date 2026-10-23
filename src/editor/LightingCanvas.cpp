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
        rendered_ = LightingDocument::render(document_.state(),seconds_,previewChannel_);
        emptyAction_->setVisible(context_==Context::Effects ? document_.state().layers.isEmpty() : document_.state().placements.isEmpty());
        update();
    });
    rendered_ = LightingDocument::render(document_.state(),seconds_,previewChannel_);
}

void LightingCanvas::fitToView()
{
    cancelGesture(); autoFit_=true; pan_={};
    zoom_=std::clamp(std::min((width()-80)/640.0,(height()-100)/400.0),0.1,4.0); update();
}
void LightingCanvas::resizeEvent(QResizeEvent*)
{
    if(autoFit_) fitToView();
    emptyAction_->move((width()-emptyAction_->width())/2,height()/2+32);
}

void LightingCanvas::setPreviewFrame(const QImage& frame, double seconds, const QUuid& channel)
{
    seconds_ = seconds; previewChannel_=channel; rendered_ = frame; update();
}

QPointF LightingCanvas::offset() const
{
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
    dragging_ = false;
    panning_ = false;
    workingMask_ = {}; originalEffect_.reset(); workingEffect_.reset(); handle_ = 0;
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
        QString("%1    /    640 x 400    /    %2%").arg(context_==Context::Layout ? "LAYOUT" : "EFFECT CANVAS").arg(qRound(zoom_*100)));
    const bool empty=context_==Context::Effects ? document_.state().layers.isEmpty() : document_.state().placements.isEmpty();
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
    painter.drawRect(QRectF(QPointF(),LightingDocument::canvasSize()).adjusted(-1,-1,1,1));
    painter.setClipRect(QRect(QPoint(), LightingDocument::canvasSize()));
    for (int y = 0; y < 400; y += 16)
        for (int x = 0; x < 640; x += 16)
            painter.fillRect(x, y, 16, 16, (x / 16 + y / 16) % 2 ? QColor("#484b50") : QColor("#35383d"));
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
        if (!maskTarget_) for (const auto& stored : document_.state().layers) if (stored.id==selected_ && stored.effect!=EffectKind::StaticColor) {
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
    // Layout previews the current effect, but only physical placements are editable.
    painter.drawImage(0, 0, frame);
    painter.setRenderHint(QPainter::Antialiasing);
    for (auto placement : document_.state().placements) {
        if (dragging_ && placement.id == selected_) placement.origin = origin_ + last_ - start_;
        const auto points = componentPoints(placement);
        const auto& definition = document_.catalog()[placement.componentId];
        painter.setPen(QPen(placement.id == selected_ ? QColor("#a9c2ff") : Qt::white, 1.5 / zoom_));
        for (int i = 0; i < points.size(); ++i) {
            const auto point = points[i];
            if (!QRectF(frame.rect()).adjusted(-20, -20, 20, 20).contains(point)) continue;
            const QColor light = QRectF(frame.rect()).contains(point) ? frame.pixelColor(point.toPoint()) : Qt::black;
            painter.setBrush(light);
            painter.drawEllipse(point, 6, 6);
            painter.drawText(point + QPointF(7, -7), QString::number(definition.mapping[i]));
        }
        if (QRectF(frame.rect()).contains(placement.origin))
            painter.drawText(placement.origin + QPointF(0, -22), definition.name);
    }
}

void LightingCanvas::mousePressEvent(QMouseEvent* event)
{
    setFocus();
    if (event->button() == Qt::MiddleButton) { cancelGesture(); autoFit_=false; panning_ = true; last_ = event->position(); return; }
    if (event->button() != Qt::LeftButton) return;
    start_ = last_ = canvasPoint(event->position());
    if (!QRectF(QPointF(), LightingDocument::canvasSize()).contains(start_)) return;
    if (context_ == Context::Layout) {
        QUuid hit;
        for (auto it = document_.state().placements.crbegin(); it != document_.state().placements.crend(); ++it) {
            for (auto point : componentPoints(*it))
                if (QLineF(point, start_).length() < 12 / zoom_) { hit = it->id; origin_ = it->origin; break; }
            if (!hit.isNull()) break;
        }
        select(hit);
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
    if (context_ == Context::Effects && maskTarget_ && tool_ == MaskTool::Brush)
        LightingDocument::paintMask(workingMask_, last_, point, radius_, hardness_, opacity_, gray_);
    last_ = point;
    if (workingEffect_) updateEffectGesture(point);
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
    if (cropTool_) {
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
    if (event->button() == Qt::MiddleButton) { panning_ = false; return; }
    if (event->button() != Qt::LeftButton || !dragging_) return;
    const auto id = selected_;
    if (context_ == Context::Layout) {
        const auto target = origin_ + last_ - start_;
        if (last_ != start_) document_.edit("Move component", [&](LightingState& state) {
            for (auto& placement : state.placements) if (placement.id == id) placement.origin = target;
        });
    } else if (workingEffect_ && last_!=start_) {
        const auto effect=*workingEffect_;
        document_.edit(cropTool_ ? "Crop image" : "Transform effect",[&](auto& state) {
            for (auto& layer : state.layers) if (layer.id==id) {
                layer.center=effect.center; layer.extent=effect.extent; layer.rotation=effect.rotation; layer.crop=effect.crop;
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
    cancelGesture();
}

void LightingCanvas::wheelEvent(QWheelEvent* event)
{
    autoFit_=false;
    cancelGesture();
    const auto before = canvasPoint(event->position());
    zoom_ = std::clamp(zoom_ * std::pow(1.15, event->angleDelta().y() / 120.0), 0.25, 4.0);
    pan_ += event->position() - (before * zoom_ + offset());
    update();
}

void LightingCanvas::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) { cancelGesture(); return; }
    if (context_ == Context::Layout && !selected_.isNull()) {
        QPointF delta;
        if (event->key() == Qt::Key_Left) delta.setX(-1);
        if (event->key() == Qt::Key_Right) delta.setX(1);
        if (event->key() == Qt::Key_Up) delta.setY(-1);
        if (event->key() == Qt::Key_Down) delta.setY(1);
        if (!delta.isNull()) {
            if (event->modifiers() & Qt::ShiftModifier) delta *= 10;
            document_.edit("Nudge component", [&](LightingState& state) {
                for (auto& placement : state.placements) if (placement.id == selected_) placement.origin += delta;
            });
            return;
        }
    }
    QWidget::keyPressEvent(event);
}
void LightingCanvas::hideEvent(QHideEvent* event) { cancelGesture(); QWidget::hideEvent(event); }
