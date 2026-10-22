#pragma once
#include "device/Component.h"
#include <QPainter>
#include <QSvgRenderer>
#include <map>
#include <cmath>
#include <memory>

inline QString hardwareLightingDescription(const Component& c) {
    if(c.type.find("count required")!=std::string::npos)return "LED address count unverified / configure before adding";
    if(c.type=="RAM")return QString("%1 addressable light-bar zones / SMBus controller").arg(c.ledCount);
    if(c.type=="GPU" || c.type=="Motherboard")return "1 shared color zone";
    return QString("%1 addresses / %2 physical LEDs").arg(c.ledCount).arg(c.ledCount*c.physicalLedsPerAddress);
}
inline QString compactHardwareName(const Component& c) {
    QString name=QString::fromStdString(c.displayName);
    if(name.startsWith("Generic fan"))name=name.contains(" #")?"Fan"+name.mid(name.lastIndexOf(" #")):QString("Fan / %1 LEDs / %2 regions").arg(c.ledCount*c.physicalLedsPerAddress).arg(c.ledCount);
    else if(name.startsWith("Generic strip"))name=name.contains(" #")?"Strip"+name.mid(name.lastIndexOf(" #")):QString("Strip / %1 LEDs").arg(c.ledCount*c.physicalLedsPerAddress);
    return name;
}

// Original generic illustrations, not vendor product images. Keep one cached
// SVG renderer per kind; reparsing artwork on every preview frame causes jitter.
inline QString deviceArtworkKind(const Component& component) {
    const auto type=QString::fromStdString(component.type).toLower();
    if(type.contains("ram"))return "ram";
    if(type.contains("motherboard"))return "motherboard";
    if(type.contains("fan"))return "fan";
    if(type=="keyboardzones")return "keyboard-zones";
    if(type.contains("keyboard"))return "keyboard";
    if(type.contains("mouse"))return "mouse";
    if(type.contains("strip") || component.productName=="Generic Placeholder")return "strip";
    if(type.contains("aio") || type.contains("cooler"))return "aio";
    if(type.contains("water") || type.contains("pump"))return "waterblock";
    if(type.contains("case") || type.contains("tower"))return "case";
    if(type.contains("cable"))return "cable";
    if(type.contains("gpu"))return "gpu";
    if(type.contains("heatsink"))return "heatsink";
    if(type.contains("desk"))return "desk";
    if(type.contains("chair"))return "chair";
    return "controller";
}
inline QString hardwareCategory(const Component& c) {
    const auto kind=deviceArtworkKind(c);
    if(kind=="ram")return "RAM";if(kind=="motherboard")return "Motherboards";
    if(kind=="fan")return "Fans";if(kind=="aio")return "AIO / coolers";if(kind=="strip")return "LED strips";
    if(kind=="waterblock")return "Waterblocks / pumps";if(kind=="case")return "Cases";if(kind=="cable")return "Cables";
    if(kind=="gpu")return "Graphics cards";if(kind=="heatsink")return "Heatsinks / GPU";if(kind=="desk" || kind=="chair")return "Furniture";return "Other";
}

inline void drawDeviceArtwork(QPainter& painter,const QRectF& rect,const Component& component) {
    static std::map<QString,std::unique_ptr<QSvgRenderer>> renderers;
    const auto kind=deviceArtworkKind(component);auto& renderer=renderers[kind];
    if(!renderer)renderer=std::make_unique<QSvgRenderer>(":/studio/devices/"+kind+".svg");
    if(component.layoutDefaults.size()==component.ledCoordinates.size() && component.ledCount>1) {
        // LED coordinates are baked on edit; derive the illustration rotation from
        // the reset geometry so symmetric fans still show their individual rotation.
        const auto a=component.layoutDefaults[0],b=component.ledCoordinates[0];int far=1;double distance=0;
        for(int i=1;i<component.ledCount;++i){const auto c=component.layoutDefaults[i];const double d=std::hypot(c.first-a.first,c.second-a.second);if(d>distance){distance=d;far=i;}}
        const auto c=component.layoutDefaults[far],d=component.ledCoordinates[far];
        const double angle=std::atan2(d.second-b.second,d.first-b.first)-std::atan2(c.second-a.second,c.first-a.first);
        if(distance>0 && std::abs(angle)>.001){
            double minX=1e9,minY=1e9,maxX=-1e9,maxY=-1e9,rx0=1e9,ry0=1e9,rx1=-1e9,ry1=-1e9;
            for(const auto& [x,y]:component.ledCoordinates){minX=std::min(minX,double(x));maxX=std::max(maxX,double(x));minY=std::min(minY,double(y));maxY=std::max(maxY,double(y));const double rx=x*std::cos(angle)+y*std::sin(angle),ry=-x*std::sin(angle)+y*std::cos(angle);rx0=std::min(rx0,rx);rx1=std::max(rx1,rx);ry0=std::min(ry0,ry);ry1=std::max(ry1,ry);}
            const QSizeF rawSize(std::max(12.,rect.width()*(rx1-rx0)/std::max(1.,maxX-minX)),std::max(12.,rect.height()*(ry1-ry0)/std::max(1.,maxY-minY)));
            const double side=std::min(rawSize.width(),rawSize.height());const QSizeF size=kind=="fan"?QSizeF(side,side):rawSize;
            painter.save();painter.translate(rect.center());painter.rotate(angle*180./3.141592653589793);renderer->render(&painter,QRectF(QPointF(-size.width()/2,-size.height()/2),size));painter.restore();return;
        }
    }
    if(kind=="fan"){
        const double side=std::min(rect.width(),rect.height());renderer->render(&painter,QRectF(rect.center()-QPointF(side/2,side/2),QSizeF(side,side)));
    }else renderer->render(&painter,rect);
}
