#pragma once
#include "device/Component.h"
#include <QPainter>
#include <QSvgRenderer>
#include <map>
#include <memory>

// Original generic illustrations, not vendor product images. Keep one cached
// SVG renderer per kind; reparsing artwork on every preview frame causes jitter.
inline QString deviceArtworkKind(const Component& component) {
    const auto type=QString::fromStdString(component.type).toLower();
    if(type.contains("fan"))return "fan";
    if(type=="keyboardzones")return "keyboard-zones";
    if(type.contains("keyboard"))return "keyboard";
    if(type.contains("mouse"))return "mouse";
    if(type.contains("strip") || component.productName=="Generic Placeholder")return "strip";
    return "controller";
}
inline void drawDeviceArtwork(QPainter& painter,const QRectF& rect,const Component& component) {
    static std::map<QString,std::unique_ptr<QSvgRenderer>> renderers;
    const auto kind=deviceArtworkKind(component);auto& renderer=renderers[kind];
    if(!renderer)renderer=std::make_unique<QSvgRenderer>(":/studio/devices/"+kind+".svg");
    renderer->render(&painter,rect);
}
