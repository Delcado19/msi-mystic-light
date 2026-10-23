#include "EffectRenderer.h"
#include "LightingDocument.h"
#include <QPainter>
#include <QPainterPath>
#include <QTransform>
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {
constexpr double pi = 3.14159265358979323846;
QPainterPath polygon(int vertices, double inner = 0)
{
    QPainterPath path;
    const int count = inner > 0 ? vertices * 2 : vertices;
    for (int i = 0; i < count; ++i) {
        const double angle = 2 * pi * i / count - pi / 2;
        const double radius = inner > 0 && i % 2 ? inner : 0.5;
        const QPointF point(radius * std::cos(angle), radius * std::sin(angle));
        if (i == 0) path.moveTo(point); else path.lineTo(point);
    }
    path.closeSubpath();
    return path;
}
QRgb interpolate(QRgb from, QRgb to, double t)
{
    // Interpolate premultiplied channels, avoiding color halos at transparent stops.
    return qRgba(qRound(qRed(from) + (qRed(to) - qRed(from)) * t),
        qRound(qGreen(from) + (qGreen(to) - qGreen(from)) * t),
        qRound(qBlue(from) + (qBlue(to) - qBlue(from)) * t),
        qRound(qAlpha(from) + (qAlpha(to) - qAlpha(from)) * t));
}
}

const QVector<EffectDefinition>& effectDefinitions()
{
    static const QVector<EffectDefinition> definitions{
        {EffectKind::StaticColor, "static", "Static Color"},
        {EffectKind::LinearGradient, "linear", "Linear Gradient"},
        {EffectKind::RadialGradient, "radial", "Radial Gradient"},
        {EffectKind::ConicalGradient, "conical", "Conical Gradient"},
        {EffectKind::Stripes, "stripes", "Stripes"}, {EffectKind::Waves, "waves", "Waves"},
        {EffectKind::Circle, "circle", "Circle"}, {EffectKind::Ring, "ring", "Ring"},
        {EffectKind::Star, "star", "Star"}, {EffectKind::Heart, "heart", "Heart"},
        {EffectKind::Triangle, "triangle", "Triangle"}, {EffectKind::Hexagon, "hexagon", "Hexagon"},
        {EffectKind::Diamond, "diamond", "Diamond"}, {EffectKind::Cross, "cross", "Cross"},
        {EffectKind::Moon, "moon", "Moon"}, {EffectKind::Image, "image", "Image"}
    };
    return definitions;
}
const EffectDefinition& effectDefinition(EffectKind kind)
{
    for (const auto& definition : effectDefinitions()) if (definition.kind == kind) return definition;
    throw std::invalid_argument("Unknown effect kind");
}
const EffectDefinition& effectDefinition(const QString& id)
{
    for (const auto& definition : effectDefinitions()) if (definition.id == id) return definition;
    throw std::invalid_argument("Unknown effect type");
}
bool isGradient(EffectKind kind) { return kind >= EffectKind::LinearGradient && kind <= EffectKind::Waves; }

void validateEffect(const EffectLayer& layer)
{
    effectDefinition(layer.effect);
    if (!layer.image.isNull() && (layer.image.width() > 4096 || layer.image.height() > 4096))
        throw std::invalid_argument("Image exceeds 4096 pixels per side");
    if (!layer.color.isValid() || !layer.secondaryColor.isValid() ||
        !std::isfinite(layer.center.x()) || !std::isfinite(layer.center.y()) ||
        std::abs(layer.center.x()) > 10000 || std::abs(layer.center.y()) > 10000 ||
        !std::isfinite(layer.extent.width()) || !std::isfinite(layer.extent.height()) ||
        layer.extent.width() < 1 || layer.extent.width() > 10000 ||
        layer.extent.height() < 1 || layer.extent.height() > 10000 ||
        !std::isfinite(layer.rotation) || layer.rotation < -360 || layer.rotation > 360 ||
        layer.repetitions < 1 || layer.repetitions > 64)
        throw std::invalid_argument("Invalid effect parameters");
}

QImage renderEffect(const EffectLayer& layer)
{
    validateEffect(layer);
    QImage image(LightingDocument::canvasSize(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    if (layer.effect == EffectKind::StaticColor) { image.fill(layer.color); return image; }
    if (layer.effect == EffectKind::Image) {
        if (layer.image.isNull()) return image;
        QPainter painter(&image);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.translate(layer.center);
        painter.rotate(layer.rotation);
        const QRectF frame(-layer.extent.width()/2, -layer.extent.height()/2,
            layer.extent.width(), layer.extent.height());
        painter.setClipRect(frame);
        const QSizeF size = QSizeF(layer.image.size()).scaled(layer.extent,
            layer.imageFill ? Qt::KeepAspectRatioByExpanding : Qt::KeepAspectRatio);
        painter.drawImage(QRectF(QPointF(-size.width()/2, -size.height()/2), size), layer.image);
        return image;
    }
    if (isGradient(layer.effect)) {
        const double angle = layer.rotation * pi / 180;
        const double cosine = std::cos(angle), sine = std::sin(angle);
        const QRgb first = qPremultiply(layer.color.rgba()), second = qPremultiply(layer.secondaryColor.rgba());
        for (int y = 0; y < image.height(); ++y) {
            auto* row = reinterpret_cast<QRgb*>(image.scanLine(y));
            for (int x = 0; x < image.width(); ++x) {
                const double dx = x + 0.5 - layer.center.x(), dy = y + 0.5 - layer.center.y();
                const double u = (dx * cosine + dy * sine) / layer.extent.width();
                const double v = (-dx * sine + dy * cosine) / layer.extent.height();
                double t = u + 0.5;
                switch (layer.effect) {
                case EffectKind::RadialGradient: t = 2 * std::hypot(u, v); break;
                case EffectKind::ConicalGradient:
                    t = std::atan2(v, u) / (2 * pi); if (t < 0) t += 1; break;
                case EffectKind::Stripes: {
                    const double phase = t * layer.repetitions;
                    t = phase - std::floor(phase) < 0.5 ? 0 : 1; break;
                }
                case EffectKind::Waves: t = (1 - std::cos(2 * pi * t * layer.repetitions)) / 2; break;
                default: break;
                }
                row[x] = interpolate(first, second, std::clamp(t, 0.0, 1.0));
            }
        }
        return image;
    }
    QPainterPath path;
    switch (layer.effect) {
    case EffectKind::Circle: path.addEllipse(QRectF(-0.5, -0.5, 1, 1)); break;
    case EffectKind::Ring:
        path.addEllipse(QRectF(-0.5, -0.5, 1, 1));
        path.addEllipse(QRectF(-0.325, -0.325, 0.65, 0.65)); break;
    case EffectKind::Star: path = polygon(5, 0.22); break;
    case EffectKind::Heart:
        path.moveTo(0, 0.5);
        path.cubicTo(-0.9, -0.1, -0.35, -0.8, 0, -0.25);
        path.cubicTo(0.35, -0.8, 0.9, -0.1, 0, 0.5); path.closeSubpath(); break;
    case EffectKind::Triangle: path = polygon(3); break;
    case EffectKind::Hexagon: path = polygon(6); break;
    case EffectKind::Diamond: path = polygon(4); break;
    case EffectKind::Cross: {
        QPainterPath horizontal, vertical;
        horizontal.addRect(-0.5, -0.15, 1, 0.3); vertical.addRect(-0.15, -0.5, 0.3, 1);
        path = horizontal.united(vertical); break;
    }
    case EffectKind::Moon: {
        QPainterPath disc, cutout;
        disc.addEllipse(QRectF(-0.5, -0.5, 1, 1)); cutout.addEllipse(QRectF(-0.15, -0.6, 1, 1));
        path = disc.subtracted(cutout); break;
    }
    default: throw std::invalid_argument("Effect is not a shape");
    }
    QTransform transform;
    transform.translate(layer.center.x(), layer.center.y());
    transform.rotate(layer.rotation);
    transform.scale(layer.extent.width(), layer.extent.height());
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillPath(transform.map(path), layer.color);
    return image;
}
