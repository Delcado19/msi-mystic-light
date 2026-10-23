#include "LightingDocument.h"
#include <QPainter>
#include <QUndoCommand>
#include <QSet>
#include <QBuffer>
#include <QFile>
#include <QSaveFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QImageReader>
#include <cmath>
#include <stdexcept>
#include <algorithm>

namespace {
class DocumentEdit : public QUndoCommand {
public:
    DocumentEdit(const QString& label, std::function<void(bool)> apply)
        : QUndoCommand(label), apply_(std::move(apply)) {}
    void undo() override { apply_(false); }
    void redo() override { apply_(true); }
private:
    std::function<void(bool)> apply_;
};
void validate(const LightingState& state)
{
    if (state.layers.size() > 64 || state.placements.size() > 256)
        throw std::invalid_argument("Document limit: 64 effect layers and 256 component placements");
    QSet<QUuid> ids;
    for (const auto& layer : state.layers) {
        if (layer.id.isNull() || ids.contains(layer.id) || layer.name.trimmed().isEmpty() ||
            !layer.color.isValid() || layer.transparency < 0 || layer.transparency > 100 ||
            layer.blend < LayerBlend::Normal || layer.blend > LayerBlend::Overlay ||
            layer.mask.size() != LightingDocument::canvasSize() || layer.mask.format() != QImage::Format_Grayscale8)
            throw std::invalid_argument("Invalid effect layer");
        ids.insert(layer.id);
    }
    for (const auto& placement : state.placements) {
        if (placement.id.isNull() || ids.contains(placement.id) ||
            !std::isfinite(placement.origin.x()) || !std::isfinite(placement.origin.y()))
            throw std::invalid_argument("Invalid component placement");
        ids.insert(placement.id);
    }
}
}

LightingDocument::LightingDocument(ComponentCatalog catalog, QObject* parent)
    : QObject(parent), catalog_(std::move(catalog))
{
    // Raster snapshots share storage until modified; bound retained mask history.
    history_.setUndoLimit(40);
}

void LightingDocument::edit(const QString& label, const std::function<void(LightingState&)>& operation)
{
    auto after = state_;
    operation(after);
    validate(after);
    for (const auto& placement : after.placements)
        if (!catalog_.contains(placement.componentId)) throw std::invalid_argument("Unknown component");
    history_.push(new DocumentEdit(label, [this, before = state_, after](bool forward) {
        state_ = forward ? after : before;
        emit changed();
    }));
}

QImage LightingDocument::whiteMask()
{
    QImage mask(canvasSize(), QImage::Format_Grayscale8);
    mask.fill(255);
    return mask;
}

QUuid LightingDocument::addLayer()
{
    EffectLayer layer;
    layer.mask = whiteMask();
    edit("Add effect layer", [&](LightingState& state) { state.layers.append(layer); });
    return layer.id;
}

QUuid LightingDocument::addComponent(const QString& componentId)
{
    ComponentPlacement placement;
    placement.componentId = componentId;
    edit("Add component", [&](LightingState& state) { state.placements.append(placement); });
    return placement.id;
}

QImage LightingDocument::render() const { return render(state_); }

void LightingDocument::save(const QString& fileName)
{
    QJsonArray layers, placements;
    for (const auto& layer : state_.layers) {
        QByteArray png;
        QBuffer buffer(&png); buffer.open(QIODevice::WriteOnly);
        if (!layer.mask.save(&buffer, "PNG")) throw std::runtime_error("Could not encode layer mask");
        layers.append(QJsonObject{{"id", layer.id.toString()}, {"name", layer.name},
            {"color", layer.color.name(QColor::HexArgb)}, {"visible", layer.visible},
            {"transparency", layer.transparency}, {"blend", static_cast<int>(layer.blend)},
            {"maskEnabled", layer.maskEnabled}, {"maskPng", QString::fromLatin1(png.toBase64())}});
    }
    for (const auto& placement : state_.placements)
        placements.append(QJsonObject{{"id", placement.id.toString()}, {"componentId", placement.componentId},
            {"x", placement.origin.x()}, {"y", placement.origin.y()}});
    const auto bytes = QJsonDocument(QJsonObject{{"schemaVersion", 1}, {"width", 640}, {"height", 400},
        {"layers", layers}, {"placements", placements}}).toJson();
    QSaveFile file(fileName);
    // Atomic replacement preserves an existing project if writing/commit fails.
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit())
        throw std::runtime_error("Could not save lighting document");
    history_.setClean();
}

void LightingDocument::load(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly) || file.size() > 32 * 1024 * 1024)
        throw std::invalid_argument("Document unreadable or larger than 32 MiB");
    QJsonParseError error;
    const auto parsed = QJsonDocument::fromJson(file.read(32 * 1024 * 1024 + 1), &error);
    const auto root = parsed.object();
    if (error.error != QJsonParseError::NoError || !parsed.isObject() ||
        root.value("schemaVersion").toDouble(-1) != 1 || root.value("width").toDouble(-1) != 640 ||
        root.value("height").toDouble(-1) != 400 || !root.value("layers").isArray() || !root.value("placements").isArray())
        throw std::invalid_argument("Invalid lighting document header");
    const auto layerArray = root.value("layers").toArray();
    const auto placementArray = root.value("placements").toArray();
    if (layerArray.size() > 64 || placementArray.size() > 256)
        throw std::invalid_argument("Document exceeds layer/component limits");
    LightingState candidate;
    for (const auto& entry : layerArray) {
        const auto json = entry.toObject();
        const double transparency = json.value("transparency").toDouble(-1);
        const double blend = json.value("blend").toDouble(-1);
        if (!json.value("visible").isBool() || !json.value("maskEnabled").isBool() ||
            transparency < 0 || transparency > 100 || std::floor(transparency) != transparency ||
            blend < 0 || blend > 3 || std::floor(blend) != blend)
            throw std::invalid_argument("Invalid layer properties");
        EffectLayer layer;
        layer.id = QUuid(json.value("id").toString()); layer.name = json.value("name").toString();
        layer.color = QColor(json.value("color").toString()); layer.visible = json.value("visible").toBool();
        layer.transparency = static_cast<int>(transparency); layer.blend = static_cast<LayerBlend>(static_cast<int>(blend));
        layer.maskEnabled = json.value("maskEnabled").toBool();
        const auto encoded = json.value("maskPng").toString().toLatin1();
        const auto decoded = QByteArray::fromBase64Encoding(encoded, QByteArray::AbortOnBase64DecodingErrors);
        if (!decoded) throw std::invalid_argument("Invalid encoded mask");
        QBuffer buffer; buffer.setData(decoded.decoded); buffer.open(QIODevice::ReadOnly);
        QImageReader reader(&buffer, "PNG");
        // Check dimensions before decoding to avoid unbounded raster allocation.
        if (reader.size() != canvasSize()) throw std::invalid_argument("Invalid mask dimensions");
        const auto mask = reader.read();
        if (mask.isNull() || !mask.isGrayscale() || mask.hasAlphaChannel()) throw std::invalid_argument("Invalid grayscale mask");
        layer.mask = mask.convertToFormat(QImage::Format_Grayscale8);
        candidate.layers.append(layer);
    }
    for (const auto& entry : placementArray) {
        const auto json = entry.toObject();
        if (!json.value("x").isDouble() || !json.value("y").isDouble())
            throw std::invalid_argument("Invalid placement coordinates");
        ComponentPlacement placement;
        placement.id = QUuid(json.value("id").toString());
        placement.componentId = json.value("componentId").toString();
        placement.origin = {json.value("x").toDouble(), json.value("y").toDouble()};
        if (!catalog_.contains(placement.componentId)) throw std::invalid_argument("Document requires an unavailable component");
        candidate.placements.append(placement);
    }
    validate(candidate);
    // Parse/validate everything before replacing the live document and undo history.
    state_ = std::move(candidate);
    history_.clear();
    history_.setClean();
    emit changed();
}

void LightingDocument::paintMask(QImage& mask, QPointF from, QPointF to,
    double radius, double hardness, double opacity, int gray)
{
    if (mask.size() != canvasSize() || mask.format() != QImage::Format_Grayscale8 ||
        !std::isfinite(from.x()) || !std::isfinite(from.y()) ||
        !std::isfinite(to.x()) || !std::isfinite(to.y()) ||
        !std::isfinite(radius) || radius < 1 || radius > 160 ||
        !std::isfinite(hardness) || hardness < 0 || hardness > 1 ||
        !std::isfinite(opacity) || opacity < 0 || opacity > 1 || gray < 0 || gray > 255)
        throw std::invalid_argument("Invalid mask brush");
    const auto segment = to - from;
    const double lengthSquared = QPointF::dotProduct(segment, segment);
    const QRect bounds = QRectF(from, to).normalized().adjusted(-radius, -radius, radius, radius)
        .toAlignedRect().intersected(mask.rect());
    for (int y = bounds.top(); y <= bounds.bottom(); ++y) {
        auto* row = mask.scanLine(y);
        for (int x = bounds.left(); x <= bounds.right(); ++x) {
            const QPointF point(x + 0.5, y + 0.5);
            const double t = lengthSquared > 0 ? std::clamp(QPointF::dotProduct(point - from, segment) / lengthSquared, 0.0, 1.0) : 0;
            const auto distance = point - (from + t * segment);
            const double d = std::hypot(distance.x(), distance.y()) / radius;
            const double coverage = d >= 1 ? 0 : d <= hardness ? 1 : (1 - d) / (1 - hardness);
            row[x] = static_cast<uchar>(qRound(row[x] + (gray - row[x]) * coverage * opacity));
        }
    }
}

void LightingDocument::gradientMask(QImage& mask, QPointF from, QPointF to)
{
    if (mask.size() != canvasSize() || mask.format() != QImage::Format_Grayscale8 ||
        !std::isfinite(from.x()) || !std::isfinite(from.y()) ||
        !std::isfinite(to.x()) || !std::isfinite(to.y()))
        throw std::invalid_argument("Invalid mask gradient");
    const auto axis = to - from;
    const double lengthSquared = QPointF::dotProduct(axis, axis);
    if (lengthSquared < 1) return;
    for (int y = 0; y < mask.height(); ++y) {
        auto* row = mask.scanLine(y);
        for (int x = 0; x < mask.width(); ++x)
            row[x] = static_cast<uchar>(qRound(255 * std::clamp(
                QPointF::dotProduct(QPointF(x + 0.5, y + 0.5) - from, axis) / lengthSquared, 0.0, 1.0)));
    }
}

QImage LightingDocument::render(const LightingState& state)
{
    validate(state);
    QImage result(canvasSize(), QImage::Format_ARGB32_Premultiplied);
    result.fill(Qt::transparent);
    QPainter painter(&result);
    for (const auto& layer : state.layers) {
        if (!layer.visible || layer.transparency == 100) continue;
        QImage source(canvasSize(), QImage::Format_ARGB32_Premultiplied);
        for (int y = 0; y < source.height(); ++y) {
            auto* pixels = reinterpret_cast<QRgb*>(source.scanLine(y));
            const auto* mask = layer.mask.constScanLine(y);
            for (int x = 0; x < source.width(); ++x) {
                const int coverage = layer.maskEnabled ? mask[x] : 255;
                const int alpha = qRound(layer.color.alphaF() * (100 - layer.transparency) / 100.0 * coverage);
                pixels[x] = qPremultiply(qRgba(layer.color.red(), layer.color.green(), layer.color.blue(), alpha));
            }
        }
        // Qt composes RGBA, so Multiply over transparent pixels retains source light.
        switch (layer.blend) {
        case LayerBlend::Normal: painter.setCompositionMode(QPainter::CompositionMode_SourceOver); break;
        case LayerBlend::Multiply: painter.setCompositionMode(QPainter::CompositionMode_Multiply); break;
        case LayerBlend::Screen: painter.setCompositionMode(QPainter::CompositionMode_Screen); break;
        case LayerBlend::Overlay: painter.setCompositionMode(QPainter::CompositionMode_Overlay); break;
        }
        painter.drawImage(0, 0, source);
    }
    return result;
}
