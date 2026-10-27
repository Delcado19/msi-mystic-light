#include "HardwareArtwork.h"
#include "HardwareIdentity.h"
#include <QPainter>
#include <QSvgRenderer>
#include <QMap>
#include <QSharedPointer>
#include <QPixmapCache>
#include <QLineF>
#include <cmath>
#include <algorithm>
#include <set>
#include <limits>

static void initializeHardwareArtwork() { Q_INIT_RESOURCE(hardware); }

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

QRectF hardwareBodyBounds(const ComponentDefinition& component)
{
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
    static const bool initialized=[] {initializeHardwareArtwork();return true;}();Q_UNUSED(initialized);
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
