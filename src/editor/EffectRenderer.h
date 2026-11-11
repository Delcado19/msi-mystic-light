#pragma once

#include <QImage>
#include <QString>
#include <QVector>
#include <QTransform>

enum class EffectKind {
    StaticColor, LinearGradient, RadialGradient, ConicalGradient, Stripes, Waves,
    Circle, Ring, Star, Heart, Triangle, Hexagon, Diamond, Cross, Moon, Image, Rain, Orbit, Fireflies, Burst, Paint, Rectangle,
    Pinwheel, Spiral, Visor, SineWave, Dna, PixelFill, Voronoi, Fireplace, Radar, Fireworks, Plasma, Ripples, Aurora, LavaLamp, Checkerboard, Kaleidoscope, Lightning, Starfield
};
struct EffectDefinition {
    EffectKind kind;
    QString id, label;
    // Source creation and Reset to default must use the same effect-specific values.
    double patternSpeed=.25, particleCount=80;
    QString description;
    double patternSites=12;
};
const QVector<EffectDefinition>& effectDefinitions();
const EffectDefinition& effectDefinition(EffectKind kind);
const EffectDefinition& effectDefinition(const QString& id);
bool isGradient(EffectKind kind);
bool isPattern(EffectKind kind);
bool isShape(EffectKind kind);
// Legacy-only sources remain readable but are omitted from new-source menus.
bool canCreateEffect(EffectKind kind);
struct EffectLayer;
struct EffectParameter {
    QString id, label;
    double EffectLayer::*member;
    double minimum, maximum;
    int schemaVersion=5; // New parameters must not become mandatory in older saved documents.
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
