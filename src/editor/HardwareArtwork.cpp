#include "HardwareArtwork.h"
#include "HardwareIdentity.h"
#include <QPainter>
#include <QSvgRenderer>
#include <QMap>
#include <QSharedPointer>
#include <QPixmapCache>
#include <QPixmap>
#include <QLineF>
#include <QtMath>
#include <cmath>
#include <algorithm>
#include <set>
#include <limits>

static void initializeHardwareArtwork()
{
    static const bool initialized=[] { Q_INIT_RESOURCE(hardware); return true; }();
    Q_UNUSED(initialized);
}

QString hardwareArtworkCategory(const ComponentDefinition& component)
{
    const auto identity=component.id+" "+component.name;
    if(identity.contains("strimer",Qt::CaseInsensitive) || identity.contains("strimmer",Qt::CaseInsensitive))return "Cables";
    const auto category=hardwareCategory(component);
    if(category!="Fans")return category;
    QVector<QPointF> points;
    for(int i=0;i<component.positions.size();++i)
        if(!component.nonPhysicalSlots.contains(component.mapping.value(i)))points.append(component.positions[i]);
    if(points.size()<3)return category;
    const auto start=points.first();QPointF axis;
    for(const auto& point:points)if(QPointF::dotProduct(point-start,point-start)>QPointF::dotProduct(axis,axis))axis=point-start;
    const double length=std::hypot(axis.x(),axis.y());
    if(length<.001)return category;
    // A custom fan instance may have been arranged as a strip; never expand a line into a square housing.
    for(const auto& point:points)if(std::abs(axis.x()*(point.y()-start.y())-axis.y()*(point.x()-start.x()))>length*.001)return category;
    return "Strips";
}

QPixmap hardwarePreviewPhoto(const ComponentDefinition& component)
{
    initializeHardwareArtwork();
    // Straight front views (owner-provided placeholders, 2026-11-26), named <part>_rgb_fv.
    static const QMap<QString,QString> photos{{"Fans","fan_120_rgb_fv"},{"Strips","led_strip_rgb_fv"},{"Cables","pc_light_cable_fv"},
        {"RAM","ram_rgb_fv"},{"Graphics cards","gpu_rgb_fv"},{"Keyboards","keyboard_rgb_fv"},
        {"Mice","mouse_rgb_fv"},{"AiO","aio_240_rgb_fv"},{"Motherboards","io_shield_rgb_fv"}};
    const auto category=hardwareArtworkCategory(component);
    auto name=photos.value(category);
    if (category=="AiO") for (const auto* length:{"120","360"})
        if (component.name.contains(length)) name=QString("aio_%1_rgb_fv").arg(length);
    if (name.isEmpty()) return {};
    // Devices redraws its thumbnail every preview frame; decode the ~2 MB source once and keep a
    // copy small enough (512x512 px budget, still 176 px at 200 % DPI) for the shared pixmap cache budget.
    const auto key="hardware-photo:"+name;
    QPixmap photo;
    if (!QPixmapCache::find(key,&photo)) {
        auto image=QImage(":/hardware/photos/"+name+".png").convertToFormat(QImage::Format_ARGB32);
        // The _fv sources sit on a square transparent canvas; crop to the visible part so wide parts
        // (keyboard, strip) fill their housing instead of shrinking to its height.
        int left=image.width(),top=image.height(),right=-1,bottom=-1;
        for (int y=0;y<image.height();++y) { const auto* line=reinterpret_cast<const QRgb*>(image.constScanLine(y));
            for (int x=0;x<image.width();++x) if (qAlpha(line[x])>8) {
                left=std::min(left,x);right=std::max(right,x);top=std::min(top,y);bottom=std::max(bottom,y); } }
        if (right>=left) image=image.copy(QRect(QPoint(left,top),QPoint(right,bottom)));
        photo=QPixmap::fromImage(image);
        // Budget by pixel count, not side length: the cropped strip (1246x91) stays sharp when zoomed.
        const double area=double(photo.width())*photo.height();
        if (area>512.*512.) photo=photo.scaled((photo.size()*std::sqrt(512.*512./area)).expandedTo({1,1}),
                                               Qt::KeepAspectRatio,Qt::SmoothTransformation);
        QPixmapCache::insert(key,photo);
    }
    return photo;
}

bool hardwareHousingFixed(const ComponentDefinition& component)
{
    const auto category=hardwareArtworkCategory(component);
    return category!="Strips" && category!="Cables";
}

QRectF hardwareBodyBounds(const ComponentDefinition& component)
{
    if(component.housing.isValid() && hardwareHousingFixed(component))return component.housing;
    QRectF bounds;bool first=true;
    for(int i=0;i<component.positions.size();++i) {
        if(component.nonPhysicalSlots.contains(component.mapping.value(i)))continue;
        const auto point=component.positions[i];
        if(first){bounds=QRectF(point,QSizeF(0,0));first=false;}
        else bounds=QRectF(QPointF(std::min(bounds.left(),point.x()),std::min(bounds.top(),point.y())),
                           QPointF(std::max(bounds.right(),point.x()),std::max(bounds.bottom(),point.y())));
    }
    const auto category=hardwareArtworkCategory(component);
    const double margin=(category=="Strips" || category=="Cables") ? .15 : .5;
    bounds=bounds.adjusted(-margin,-margin,margin,margin);
    if(category=="Cables") {
        // Legacy Strimer metadata stores one addressable row; retain a visible multi-wire cable housing.
        const auto size=bounds.width()>=bounds.height()
            ? QSizeF(bounds.width(),std::max(.7,bounds.height())) : QSizeF(std::max(.7,bounds.width()),bounds.height());
        bounds=QRectF(bounds.center()-QPointF(size.width()/2,size.height()/2),size);
    }
    if(category=="Keyboards") {
        const double width=std::max(.25,bounds.width()-2*margin),height=bounds.height()-2*margin;
        if(component.positions.size()-component.nonPhysicalSlots.size()>20 && height>0) {
            // Register the shared fullsize SVG to the actual per-key coordinate span.
            bounds=bounds.adjusted(margin,margin,-margin,-margin);
            bounds=bounds.adjusted(-width*.5/5.75,-height*.5/1.25,width*.5/5.75,height*.5/1.25);
        } else {
            const auto center=bounds.center();const double scale=width/5.75;
            bounds=QRectF(center-QPointF(6.75*scale/2,2.25*scale/2),QSizeF(6.75*scale,2.25*scale));
        }
    }
    if(category=="Graphics cards" && component.positions.size()-component.nonPhysicalSlots.size()==1) {
        // One address is the logo zone, not the physical size of the whole graphics card.
        const auto size=QSizeF(std::max(6.0,bounds.width()),std::max(2.8,bounds.height()));
        bounds=QRectF(bounds.center()-QPointF(size.width()/2,size.height()/2),size);
    }
    if(category=="Fans") {
        const double side=std::max(bounds.width(),bounds.height());
        bounds=QRectF(bounds.center()-QPointF(side/2,side/2),QSizeF(side,side));
    }
    return bounds;
}

double hardwarePreviewLedRadius(const ComponentDefinition& component,QSizeF scale)
{
    QVector<std::pair<double,double>> points;
    for(int i=0;i<component.positions.size();++i)if(!component.nonPhysicalSlots.contains(component.mapping.value(i)))
        points.append({component.positions[i].x()*20*scale.width(),component.positions[i].y()*20*scale.height()});
    std::sort(points.begin(),points.end());points.erase(std::unique(points.begin(),points.end()),points.end());
    // A closest-pair sweep bounds dense imports without quadratic work on large custom definitions.
    double distance=18.;qsizetype left=0;
    std::set<std::pair<double,double>> active;
    for(const auto& point:points) {
        while(left<points.size() && points[left].first<point.first-distance) {
            active.erase({points[left].second,points[left].first});++left;
        }
        auto it=active.lower_bound({point.second-distance,-std::numeric_limits<double>::infinity()});
        for(;it!=active.end() && it->first<=point.second+distance;++it)
            distance=std::min(distance,std::hypot(point.first-it->second,point.second-it->first));
        active.insert({point.second,point.first});
    }
    const auto body=hardwareBodyBounds(component);
    return std::min({6.,distance*.36,body.width()*20*scale.width()*.15,body.height()*20*scale.height()*.15});
}

void drawHardwareArtwork(QPainter& painter,const QString& category,const QRectF& bounds)
{
    if(!bounds.isValid() || !std::isfinite(bounds.width()) || !std::isfinite(bounds.height()))return;
    initializeHardwareArtwork();
    static const QMap<QString,QString> files{{"Fans","fan"},{"Strips","strip"},{"RAM","ram"},
        {"Graphics cards","gpu"},{"Keyboards","keyboard"},{"Mice","mouse"},{"Pumps","pump"},
        {"AiO","aio"},{"Motherboards","motherboard"},{"Cables","cable"},{"Strip LED","strip-led"}};
    const auto file=files.value(category,"other");
    static QMap<QString,QSharedPointer<QSvgRenderer>> renderers;
    auto& renderer=renderers[file];
    if(!renderer)renderer=QSharedPointer<QSvgRenderer>::create(":/hardware/"+file+".svg");
    const auto render=[&](const QRectF& target) {
        const auto transform=painter.deviceTransform();
        const int width=int(std::clamp(std::ceil(QLineF(transform.map(target.topLeft()),transform.map(target.topRight())).length()),1.,2048.));
        const int height=int(std::clamp(std::ceil(QLineF(transform.map(target.topLeft()),transform.map(target.bottomLeft())).length()),1.,2048.));
        const auto key=QString("hardware:%1:%2:%3").arg(file).arg(width).arg(height);
        QPixmap image;
        // Static SVG housings do not change with playback; retain bounded, resolution-specific raster artwork.
        if(!QPixmapCache::find(key,&image)) {
            image=QPixmap(width,height);image.fill(Qt::transparent);
            QPainter raster(&image);raster.setRenderHint(QPainter::Antialiasing);
            renderer->render(&raster,QRectF(0,0,width,height));raster.end();QPixmapCache::insert(key,image);
        }
        painter.drawPixmap(target,image,QRectF(0,0,width,height));
    };
    painter.save();
    if((category=="RAM" || category=="Strips" || category=="Cables") && bounds.height()>bounds.width()) {
        painter.translate(bounds.center());painter.rotate(90);
        render(QRectF(-bounds.height()/2,-bounds.width()/2,bounds.height(),bounds.width()));
    } else render(bounds);
    painter.restore();
}

void drawHardwareComponentArtwork(QPainter& painter,const ComponentDefinition& component)
{
    const auto category=hardwareArtworkCategory(component);const auto bounds=hardwareBodyBounds(component);
    painter.save();
    if(category=="Strips") {
        // SVG housing plus one SVG package per real LED; dense preview markers must not hide this artwork.
        drawHardwareArtwork(painter,category,bounds);
        const double half=component.positions.size()>1
            ? std::min(.10,.35*std::max(bounds.width()-.3,bounds.height()-.3)/(component.positions.size()-1)) : .10;
        for(int i=0;i<component.positions.size();++i)if(!component.nonPhysicalSlots.contains(component.mapping.value(i))) {
            const auto point=component.positions[i];
            drawHardwareArtwork(painter,"Strip LED",QRectF(point-QPointF(half,half),QSizeF(2*half,2*half)));
        }
    } else if(category=="Mice" && bounds.width()>bounds.height()) {
        // Legacy imports can bake the mouse's quarter-turn into its LED coordinates before placement rotation.
        painter.translate(bounds.center());painter.rotate(90);
        drawHardwareArtwork(painter,category,QRectF(-bounds.height()/2,-bounds.width()/2,bounds.height(),bounds.width()));
    } else drawHardwareArtwork(painter,category,bounds);
    painter.restore();
}

void drawHardwareEditorArtwork(QPainter& painter,const ComponentDefinition& component)
{
    // Owner decision 2026-11-26/27: dimmed straight-view photo for every fixed housing, keyboards included
    // (its keys need not line up exactly with the LED positions). Strips are stitched from the photo per
    // LED, cables stretch only their sleeve between the connectors (below).
    const auto category=hardwareArtworkCategory(component);
    QVector<QPointF> leds;
    for(int i=0;i<component.positions.size();++i)
        if(!component.nonPhysicalSlots.contains(component.mapping.value(i)))leds.append(component.positions[i]);
    const bool strip=category=="Strips" && leds.size()>=2;
    const auto photo=hardwareHousingFixed(component) || strip || category=="Cables" ? hardwarePreviewPhoto(component) : QPixmap();
    if(photo.isNull()) { drawHardwareComponentArtwork(painter,component); return; }
    static QPixmap dimmed; static qint64 source=0;
    if(source!=photo.cacheKey()) {
        // Slightly desaturated so LED markers and colors stay the focus.
        auto image=photo.toImage().convertToFormat(QImage::Format_ARGB32);
        for(int y=0;y<image.height();++y) { auto* line=reinterpret_cast<QRgb*>(image.scanLine(y));
            for(int x=0;x<image.width();++x) { const auto c=line[x]; const int g=qGray(c);
                line[x]=qRgba((qRed(c)+g)/2,(qGreen(c)+g)/2,(qBlue(c)+g)/2,qAlpha(c)); } }
        dimmed=QPixmap::fromImage(image); source=photo.cacheKey();
    }
    painter.save(); painter.setOpacity(.4); painter.setRenderHint(QPainter::SmoothPixmapTransform);
    if(strip) {
        // Owner decision 2026-11-27: the photo shows 11 LEDs, real strips any count. Stitch connector head,
        // one photo LED cell per real LED (scaled to the neighbour distance, so never distorted) and the end
        // cap. Fractions measured on the cropped led_strip_rgb_fv (1246 px wide, LED pitch ~97 px).
        const double w=dimmed.width(),h=dimmed.height(),pitch=.0780*w,first=.1814*w,middle=.5714*w,last=.9615*w;
        for(int i=0;i<leds.size();++i) {
            const bool end=i+1==leds.size();
            const auto dir=end ? leds[i]-leds[i-1] : leds[i+1]-leds[i];
            const double length=std::hypot(dir.x(),dir.y());
            if(length<1e-6)continue;
            painter.save(); painter.translate(leds[i]); painter.rotate(qRadiansToDegrees(std::atan2(dir.y(),dir.x())));
            painter.scale(length/pitch,length/pitch);
            const QRectF cell(end ? last-pitch/2 : middle-pitch/2,0,end ? w-(last-pitch/2) : pitch,h);
            painter.drawPixmap(QRectF(-pitch/2,-h/2,cell.width(),h),dimmed,cell);
            if(i==0) painter.drawPixmap(QRectF(-first,-h/2,first-pitch/2,h),dimmed,QRectF(0,0,first-pitch/2,h));
            painter.restore();
        }
    } else if(category=="Cables") {
        // Owner photo 2026-11-27: connectors keep their proportions, only the uniform sleeve between them
        // stretches to the cable length. Fractions measured on the cropped pc_light_cable_fv (1219 px wide).
        auto body=hardwareBodyBounds(component);
        if(body.height()>body.width()) {
            painter.translate(body.center()); painter.rotate(90);
            body=QRectF(-body.height()/2,-body.width()/2,body.height(),body.width());
        }
        const double w=dimmed.width(),h=dimmed.height(),head=.195*w,tail=.87*w,scale=body.height()/h;
        const double headWidth=head*scale,tailWidth=(w-tail)*scale,sleeve=std::max(0.,body.width()-headWidth-tailWidth);
        painter.drawPixmap(QRectF(body.left(),body.top(),headWidth,body.height()),dimmed,QRectF(0,0,head,h));
        painter.drawPixmap(QRectF(body.left()+headWidth,body.top(),sleeve,body.height()),dimmed,QRectF(head,0,tail-head,h));
        painter.drawPixmap(QRectF(body.left()+headWidth+sleeve,body.top(),tailWidth,body.height()),dimmed,QRectF(tail,0,w-tail,h));
    } else {
        const auto housing=hardwareBodyBounds(component);
        auto size=QSizeF(photo.size()); size.scale(housing.size(),Qt::KeepAspectRatio); // never stretched
        painter.drawPixmap(QRectF(housing.center()-QPointF(size.width()/2,size.height()/2),size),dimmed,dimmed.rect());
    }
    painter.restore();
}
