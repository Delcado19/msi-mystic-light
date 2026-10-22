#include "canvas/LayoutGeometry.h"
#include "effects/EffectClock.h"
#pragma once
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QTimer>
#include <functional>
#include "device/Component.h"
#include "ui/DeviceArtwork.h"
#include "canvas/Canvas.h"
#include "effects/IEffect.h"

// A hardware-addressed map: each hit target is the same slot sent to the driver.
class DeviceLedWidget : public QWidget {
public:
    explicit DeviceLedWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setObjectName("deviceLedMap"); setMinimumSize(300, 240); setMouseTracking(true);
        timer_.setTimerType(Qt::PreciseTimer);
        (void)effectTimeMs();
        connect(&timer_, &QTimer::timeout, this, [this] { update(); });
    }
    void setGeometryData(std::vector<Component> components) { components_ = std::move(components); update(); }
    void setEffect(std::shared_ptr<IEffect> effect, std::map<int,MsiColor> overrides, bool editable, CanvasPlacement placement = {}, bool global = false) {
        placement_=placement;global_=global;
        effect_ = std::move(effect); overrides_ = std::move(overrides); editable_ = editable; update();
    }
    void setPreviewPaused(bool paused) { if(paused && !paused_) pausedTime_=effectTimeMs(); paused_ = paused; if (paused) timer_.stop(); else if (isVisible()) timer_.start(16); update(); }
    std::function<void(int)> regionClicked;
protected:
    void showEvent(QShowEvent* e) override { QWidget::showEvent(e); if (!paused_) timer_.start(16); }
    void hideEvent(QHideEvent* e) override { timer_.stop(); QWidget::hideEvent(e); }
    void mouseMoveEvent(QMouseEvent* e) override {
        for (const auto& hit : hits_) if (hit.rect.contains(e->position())) { setToolTip(hit.name); return; }
        setToolTip("Hover a key or LED region to see its hardware address.");
    }
    void mousePressEvent(QMouseEvent* e) override {
        if (!editable_ || e->button() != Qt::LeftButton) return;
        for (const auto& hit : hits_) if (hit.rect.contains(e->position())) { if (regionClicked) regionClicked(hit.index); return; }
    }
    void paintEvent(QPaintEvent*) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(), QColor("#10161e")); hits_.clear();
        if (components_.empty()) { p.setPen(QColor("#a9b4c3")); p.drawText(rect(),Qt::AlignCenter,"No components configured. Choose a product above and click Add hardware."); return; }
        int minX=0,maxX=1,minY=0,maxY=1; bool first=true;
        for (const auto& c : components_) for (int i=0;i<c.ledCount;++i) {
            if (c.ledNames[i].find("Wire slot") == 0) continue;
            const auto [x,y]=c.ledCoordinates[i];
            if (first) { minX=maxX=x;minY=maxY=y;first=false; }
            minX=qMin(minX,x);maxX=qMax(maxX,x);minY=qMin(minY,y);maxY=qMax(maxY,y);
        }
        const bool keyboard = components_.size()==1 && components_[0].type=="Keyboard";
        // Only legacy placeholders wrap; configured strips retain their real geometry.
        const bool linear=components_.size()==1 && components_[0].type=="GenericPlaceholder";
        const int displayMaxX=linear?qMin(11,components_[0].ledCount-1)*10:maxX-minX;
        const int displayMaxY=linear?((components_[0].ledCount-1)/12)*10:maxY-minY;

        // Silhouettes extend beyond their LED centers; reserve room for them.
        const double padding=keyboard?2.0:linear?10.0:60.0;
        const double sx=(width()-90.0)/(displayMaxX+padding), sy=(height()-110.0)/(displayMaxY+padding);
        const double scale=qMax(0.1,qMin(sx,sy));
        const double originX=(width()-displayMaxX*scale)/2, originY=(height()-displayMaxY*scale)/2;
        const auto bounds=layoutBounds(components_);const auto frameTime=paused_?pausedTime_:effectTimeMs();
        int offset=0,total=0;for(const auto& c:components_)total+=c.ledCount;
        for (const auto& c : components_) {
            int cl=c.ledCoordinates[0].first,ct=c.ledCoordinates[0].second,cr=cl,cb=ct;
            for(int i=0;i<c.ledCount;++i){if(c.ledNames[i].find("Wire slot")==0)continue;const auto [x,y]=c.ledCoordinates[i];cl=qMin(cl,x);cr=qMax(cr,x);ct=qMin(ct,y);cb=qMax(cb,y);}
            if(!linear){const QRectF body(originX+(cl-minX)*scale,originY+(ct-minY)*scale,qMax(12.,(cr-cl)*scale),qMax(12.,(cb-ct)*scale));drawDeviceArtwork(p,body.adjusted(-body.width()*.09,-body.height()*.09,body.width()*.09,body.height()*.09),c);}
            if(!keyboard && !linear) {
                double cx=0,cy=0;int top=c.ledCoordinates.front().second;
                for(const auto& point:c.ledCoordinates){cx+=point.first;cy+=point.second;top=qMin(top,point.second);}cx/=c.ledCount;
                p.setPen(QColor("#c4d2e2"));p.setFont(font());
                const int labelWidth=qMax(24,int((cr-cl)*scale));
                p.drawText(QRectF(originX+(cx-minX)*scale-labelWidth/2.,originY+(top-minY)*scale-28,labelWidth,22),Qt::AlignCenter,
                    p.fontMetrics().elidedText(compactHardwareName(c),Qt::ElideRight,labelWidth));
            }
            for (int i=0;i<c.ledCount;++i) {
                if (c.ledNames[i].find("Wire slot")==0) continue;
                const auto [x,y]=c.ledCoordinates[i];
                QPointF center(originX+(linear?(i%12)*10:x-minX)*scale,originY+(linear?(i/12)*10:y-minY)*scale);
                const double size=keyboard?qMin(scale*.86,48.0):linear?qBound(12.0,scale*8.5,34.0):qBound(12.0,scale*10,36.0);
                double keyWidth=size,keyHeight=size;
                if(keyboard) {
                    // Vulcan ISO wire IDs also define the wider/taller keycaps.
                    if(i==0x25)keyWidth=scale*6.6; // Space spans the lower-row gap.
                    if(i==0x52)keyWidth=scale*2.4;
                    if(i==0x57){keyWidth=scale*1.9;center.rx()+=scale*.5;}
                    if(i==0x02 || i==0x03){keyWidth=scale*1.75;center.rx()+=scale*.4;}
                    if(i==0x58){keyHeight=scale*1.8;center.ry()-=scale*.4;}
                    if(i==0x82 || i==0x83){keyHeight=scale*1.8;center.ry()+=scale*.5;}
                    if(i==0x75){keyWidth=scale*1.9;center.rx()+=scale*.5;}
                }
                const QRectF r(center.x()-keyWidth/2,center.y()-keyHeight/2,keyWidth,keyHeight);
                const auto point=layoutSamplePoint(c.ledCoordinates[i],bounds,placement_,global_);
                const int address=offset+c.ledMapping[i];
                MsiColor color=effect_?(global_?effect_->colorAt(point.first,point.second,frameTime):effect_->colorForLed(point.first,point.second,frameTime,offset+c.effectAddress(i),total,c.effectAddress(i),c.ledCount)):MsiColor{};
                const auto fixed=overrides_.find(address);if(fixed!=overrides_.end())color=fixed->second;
                p.setPen(QPen(fixed!=overrides_.end()?QColor("#f5cb82"):QColor("#718196"),fixed!=overrides_.end()?2:1));
                p.setBrush(QColor(color.r,color.g,color.b));
                if(keyboard || c.type=="KeyboardZones") p.drawRoundedRect(r,4,4); else p.drawEllipse(r);
                if (c.physicalLedsPerAddress>1) { p.drawEllipse(r.translated(size*.6,0)); }
                if(keyboard) {
                    p.setBrush(QColor(12,17,24,220)); p.setPen(Qt::NoPen); p.drawRoundedRect(r.adjusted(1,1,-1,-4),3,3);
                    p.setPen(QColor("#eff5fc")); QFont f=font();f.setPixelSize(qBound(8,static_cast<int>(size/3),13));p.setFont(f);
                    p.drawText(r,Qt::AlignCenter,p.fontMetrics().elidedText(QString::fromStdString(c.ledNames[i]),Qt::ElideRight,static_cast<int>(keyWidth-3)));
                } else if(linear) {
                    p.setPen(Qt::NoPen);p.setBrush(QColor("#14202b"));p.drawRoundedRect(r.adjusted(1,1,-1,-1),2,2);
                    QFont f=font();f.setPixelSize(qBound(7,static_cast<int>(size/2.3),11));p.setFont(f);
                    p.setPen(QColor("#edf2f7"));p.drawText(r,Qt::AlignCenter,QString::number(address+1));
                } else {
                    p.setPen(QColor("#edf2f7")); p.drawText(r.adjusted(-20,size+3,20,25),Qt::AlignCenter,QString::number(address+1));
                }
                hits_.push_back({c.physicalLedsPerAddress>1?r.united(r.translated(size*.6,0)):r,address,QString::fromStdString(c.displayName+" / "+c.ledNames[i])+QString(" / address %1").arg(offset+c.ledMapping[i])});
            }
            offset+=c.ledCount;
        }
        p.setFont(font());p.setPen(QColor("#a9b4c3"));
        p.drawText(QRect(16,12,width()-32,36),Qt::AlignLeft|Qt::TextWordWrap,editable_?"Click a key or region to assign a color. Amber outline = fixed color.":effect_?"Live LED positions / numbers show wire order. Arrange components to move LEDs or groups.":"Native device effect: per-region editing is unavailable.");
        p.drawText(QRect(16,height()-38,width()-32,30),Qt::AlignLeft,(keyboard?QString("%1 keys / %2 wire slots").arg(hits_.size()).arg(offset):QString("%1 addressable regions / hover for zone names").arg(hits_.size())));
    }
private:
    struct Hit { QRectF rect; int index; QString name; };
    std::vector<Hit> hits_;
    std::vector<Component> components_;
    std::shared_ptr<IEffect> effect_;
    std::map<int,MsiColor> overrides_;
    CanvasPlacement placement_{};bool global_=false;int64_t pausedTime_=0;
    bool editable_=false,paused_=false;
    QTimer timer_;
};
