#pragma once

#include <QImage>
#include <QString>
#include <QVector>
#include <QTransform>

enum class EffectKind {
    StaticColor, LinearGradient, RadialGradient, ConicalGradient, Stripes, Waves,
    Circle, Ring, Star, Heart, Triangle, Hexagon, Diamond, Cross, Moon, Image, Rain, Orbit, Fireflies, Burst, Paint, Rectangle
};
struct EffectDefinition {
    EffectKind kind;
    QString id, label;
};
const QVector<EffectDefinition>& effectDefinitions();
const EffectDefinition& effectDefinition(EffectKind kind);
const EffectDefinition& effectDefinition(const QString& id);
bool isGradient(EffectKind kind);
bool isShape(EffectKind kind);
struct EffectLayer;
struct EffectParameter {
    QString id, label;
    double EffectLayer::*member;
    double minimum, maximum;
};
// Stable parameter IDs are also project-file keys; keep controls and validation aligned.
const QVector<EffectParameter>& advancedParameters();
bool isParticle(EffectKind kind);
QTransform effectTransform(const EffectLayer& layer, double seconds = 0);
QImage renderMask(const EffectLayer& layer, double seconds);
QImage transformedMask(const EffectLayer& layer, const QTransform& transform);
// One source raster, before layer masks/transparency/blending. No hardware I/O.
QImage renderEffect(const EffectLayer& layer, double seconds = 0);
void validateEffect(const EffectLayer& layer);
