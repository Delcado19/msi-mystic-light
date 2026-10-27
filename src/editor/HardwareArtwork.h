#pragma once
#include "components/ComponentCatalog.h"
#include <QRectF>
class QPainter;
// Visual housing only: never changes sampling coordinates, LED order or stored geometry.
QString hardwareArtworkCategory(const ComponentDefinition& component);
QRectF hardwareBodyBounds(const ComponentDefinition& component);
// Preview markers only; physical sampling positions and editor wire handles stay unchanged.
double hardwarePreviewLedRadius(const ComponentDefinition& component, QSizeF scale);
void drawHardwareArtwork(QPainter& painter,const QString& category,const QRectF& bounds);

void drawHardwareComponentArtwork(QPainter& painter,const ComponentDefinition& component);
