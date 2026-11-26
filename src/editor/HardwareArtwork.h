#pragma once
#include "components/ComponentCatalog.h"
#include <QRectF>
class QPainter;
class QPixmap;
// Visual housing only: never changes sampling coordinates, LED order or stored geometry.
QString hardwareArtworkCategory(const ComponentDefinition& component);
QPixmap hardwarePreviewPhoto(const ComponentDefinition& component);
QRectF hardwareBodyBounds(const ComponentDefinition& component);
// True when the component keeps a fixed housing frame; strips and cables grow with their LEDs.
bool hardwareHousingFixed(const ComponentDefinition& component);
// Component Editor background: the front-view photo, dimmed, fitted (aspect kept) into the housing;
// falls back to the SVG artwork for categories without a matching straight view.
void drawHardwareEditorArtwork(QPainter& painter,const ComponentDefinition& component);
// Preview markers only; physical sampling positions and editor wire handles stay unchanged.
double hardwarePreviewLedRadius(const ComponentDefinition& component, QSizeF scale);
void drawHardwareArtwork(QPainter& painter,const QString& category,const QRectF& bounds);

void drawHardwareComponentArtwork(QPainter& painter,const ComponentDefinition& component);
