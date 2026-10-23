#pragma once

#include <QImage>
#include <QString>
#include <QVector>

enum class EffectKind {
    StaticColor, LinearGradient, RadialGradient, ConicalGradient, Stripes, Waves,
    Circle, Ring, Star, Heart, Triangle, Hexagon, Diamond, Cross, Moon
};
struct EffectDefinition {
    EffectKind kind;
    QString id, label;
};
const QVector<EffectDefinition>& effectDefinitions();
const EffectDefinition& effectDefinition(EffectKind kind);
const EffectDefinition& effectDefinition(const QString& id);
bool isGradient(EffectKind kind);
struct EffectLayer;
// One source raster, before layer masks/transparency/blending. No hardware I/O.
QImage renderEffect(const EffectLayer& layer);
void validateEffect(const EffectLayer& layer);
