#include "HardwareArtwork.h"
#include "HardwareIdentity.h"
#include <QPainter>
#include <QSvgRenderer>
#include <QMap>
#include <QSharedPointer>
#include <cmath>
#include <algorithm>

static void initializeHardwareArtwork() { Q_INIT_RESOURCE(hardware); }

QString hardwareArtworkCategory(const ComponentDefinition& component)
{
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
    const double margin=category=="Strips" ? .15 : .5;
    bounds=bounds.adjusted(-margin,-margin,margin,margin);
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

void drawHardwareArtwork(QPainter& painter,const QString& category,const QRectF& bounds)
{
    if(!bounds.isValid() || !std::isfinite(bounds.width()) || !std::isfinite(bounds.height()))return;
    static const bool initialized=[] {initializeHardwareArtwork();return true;}();Q_UNUSED(initialized);
    static const QMap<QString,QString> files{{"Fans","fan"},{"Strips","strip"},{"RAM","ram"},
        {"Graphics cards","gpu"},{"Keyboards","keyboard"},{"Mice","mouse"},{"Pumps","pump"},
        {"AiO","aio"},{"Motherboards","motherboard"}};
    const auto file=files.value(category,"other");
    static QMap<QString,QSharedPointer<QSvgRenderer>> renderers;
    auto& renderer=renderers[file];
    if(!renderer)renderer=QSharedPointer<QSvgRenderer>::create(":/hardware/"+file+".svg");
    painter.save();
    if((category=="RAM" || category=="Strips") && bounds.height()>bounds.width()) {
        painter.translate(bounds.center());painter.rotate(90);
        renderer->render(&painter,QRectF(-bounds.height()/2,-bounds.width()/2,bounds.height(),bounds.width()));
    } else renderer->render(&painter,bounds);
    painter.restore();
}

void drawHardwareComponentArtwork(QPainter& painter,const ComponentDefinition& component)
{
    const auto category=hardwareArtworkCategory(component);const auto bounds=hardwareBodyBounds(component);
    painter.save();
    if(category=="Mice" && bounds.width()>bounds.height()) {
        // Legacy imports can bake the mouse's quarter-turn into its LED coordinates before placement rotation.
        painter.translate(bounds.center());painter.rotate(90);
        drawHardwareArtwork(painter,category,QRectF(-bounds.height()/2,-bounds.width()/2,bounds.height(),bounds.width()));
    } else drawHardwareArtwork(painter,category,bounds);
    painter.restore();
}
