#include "LightingDocument.h"
#include "HardwareIdentity.h"
#include <QPainter>
#include <QTransform>
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
#include <limits>

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
QVector<QPointF> placedPoints(const ComponentPlacement& placement,const ComponentDefinition& definition)
{
    if(!std::isfinite(placement.origin.x()) || !std::isfinite(placement.origin.y()) ||
       !std::isfinite(placement.scale.width()) || !std::isfinite(placement.scale.height()) ||
       placement.scale.width()<.01 || placement.scale.width()>100 || placement.scale.height()<.01 || placement.scale.height()>100 ||
       !std::isfinite(placement.rotation) || std::abs(placement.rotation)>360)
        throw std::invalid_argument("Invalid component placement");
    QTransform transform;transform.translate(placement.origin.x(),placement.origin.y());
    transform.rotate(placement.rotation);transform.scale(20*placement.scale.width(),20*placement.scale.height());
    QVector<QPointF> points;points.reserve(definition.positions.size());
    // Leave arithmetic headroom for world spans, marker margins and the maximum 4x navigation zoom.
    constexpr double limit=std::numeric_limits<double>::max()/32;
    for(const auto& local:definition.positions) {
        const auto point=transform.map(local);
        if(!std::isfinite(point.x()) || !std::isfinite(point.y()) || std::abs(point.x())>limit || std::abs(point.y())>limit)
            throw std::invalid_argument("Component coordinates exceed the navigable range");
        points.append(point);
    }
    return points;
}

void validate(const LightingState& state)
{
    if (state.layers.size() > 64 || state.placements.size() > 256)
        throw std::invalid_argument("Document limit: 64 effect layers and 256 component placements");
    qint64 imagePixels = 0;
    QSet<QUuid> ids;
    for (const auto& layer : state.layers) {
        validateEffect(layer);
        imagePixels += qint64(layer.image.width()) * layer.image.height();
        if (imagePixels > 16 * 1024 * 1024)
            throw std::invalid_argument("Document image budget exceeds 16 megapixels");
        if (layer.id.isNull() || ids.contains(layer.id) || layer.name.trimmed().isEmpty() ||
            !layer.color.isValid() || layer.transparency < 0 || layer.transparency > 100 ||
            layer.blend < LayerBlend::Normal || layer.blend > LayerBlend::Overlay ||
            layer.mask.size() != LightingDocument::canvasSize() || layer.mask.format() != QImage::Format_Grayscale8)
            throw std::invalid_argument("Invalid effect layer");
        ids.insert(layer.id);
    }
    for (const auto& placement : state.placements) {
        if (placement.id.isNull() || ids.contains(placement.id) || !state.components.contains(placement.componentId))
            throw std::invalid_argument("Invalid component placement");
        placedPoints(placement,state.components[placement.componentId]);
        ids.insert(placement.id);
    }
}
}

LightingDocument::LightingDocument(ComponentCatalog catalog, QObject* parent, QVector<DeviceProfile> profiles)
    : QObject(parent), profiles_(std::move(profiles)), initialCatalog_(std::move(catalog))
{
    if (initialCatalog_.size() > 256) throw std::invalid_argument("Catalog limit: 256 definitions");
    state_.components = initialCatalog_;
    // Raster snapshots share storage until modified; bound retained mask history.
    history_.setUndoLimit(40);
}

void LightingDocument::edit(const QString& label, const std::function<void(LightingState&)>& operation)
{
    auto after = state_;
    operation(after);
    validate(after);
    // Apply linked mask geometry once per undoable edit; previews never bake animation into pixels.
    for (auto& layer : after.layers) if (layer.maskLinked) {
        for (const auto& before : state_.layers) if (before.id == layer.id &&
            (before.center != layer.center || before.extent != layer.extent || before.rotation != layer.rotation)) {
            layer.mask = transformedMask(before, effectTransform(before).inverted()*effectTransform(layer));
        }
    }
    validate(after);
    for (const auto& placement : after.placements)
        if (!after.components.contains(placement.componentId)) throw std::invalid_argument("Unknown component");
    validateChannelAssignments(after, profiles_);
    history_.push(new DocumentEdit(label, [this, before = state_, after](bool forward) {
        state_ = forward ? after : before;
        ++revision_; emit changed();
    }));
}

QImage LightingDocument::whiteMask()
{
    QImage mask(canvasSize(), QImage::Format_Grayscale8);
    mask.fill(255);
    return mask;
}

QUuid LightingDocument::addLayer(EffectKind effect)
{
    EffectLayer layer;
    layer.effect = effect;
    layer.name = effectDefinition(effect).label;
    if (isGradient(effect)) layer.extent = canvasSize();
    layer.mask = whiteMask();
    edit("Add effect layer", [&](LightingState& state) { state.layers.append(layer); });
    return layer.id;
}

QUuid LightingDocument::addPaintLayer()
{
    EffectLayer layer; layer.effect=EffectKind::Paint; layer.name="Layer "+QString::number(state_.layers.size()+1);
    layer.extent=canvasSize(); layer.mask=whiteMask();
    // A null source is a transparent canvas until the first drawing gesture.
    edit("New layer",[&](LightingState& state) { state.layers.append(layer); });
    return layer.id;
}

QUuid LightingDocument::importImage(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly) || file.size() > 16 * 1024 * 1024)
        throw std::invalid_argument("Image unreadable or larger than 16 MiB");
    QImageReader reader(&file);
    if (!QList<QByteArray>{"png", "jpeg", "jpg", "bmp", "gif", "webp"}.contains(reader.format()))
        throw std::invalid_argument("Supported image formats: PNG, JPEG, BMP, GIF, WebP");
    const auto size = reader.size();
    if (!size.isValid() || size.width() > 4096 || size.height() > 4096)
        throw std::invalid_argument("Image must be at most 4096 pixels per side");
    reader.setAutoTransform(true);
    // shortcut: animated inputs import the first frame; add playback with the motion timeline.
    const auto image = reader.read();
    if (image.isNull()) throw std::invalid_argument("Unsupported or corrupt image");
    EffectLayer layer;
    layer.effect = EffectKind::Image; layer.name = "Image";
    layer.image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    layer.extent = canvasSize(); layer.mask = whiteMask();
    edit("Import image", [&](LightingState& state) { state.layers.append(layer); });
    return layer.id;
}

void LightingDocument::defineComponent(const ComponentDefinition& component, bool replace)
{
    // Use the same trust boundary for GUI-created, imported and embedded definitions.
    const auto validated = ComponentDefinition::fromJson(component.toJson());
    if (catalog().contains(validated.id) != replace)
        throw std::invalid_argument(replace ? "Unknown component ID" : "Duplicate component ID");
    if (!replace && catalog().size() >= 256) throw std::invalid_argument("Catalog limit: 256 definitions");
    edit(replace ? "Edit component definition" : "Add component definition", [&](LightingState& state) {
        state.components.insert(validated.id, validated);
    });
}

void LightingDocument::setComponentGeometry(const QUuid& id, const QVector<QPointF>& positions)
{
    edit("Edit device LED arrangement", [&](LightingState& state) {
        auto placement = std::find_if(state.placements.begin(), state.placements.end(),
            [&](const auto& item) { return item.id == id; });
        if (placement == state.placements.end()) throw std::invalid_argument("Select a component first");
        auto definition = state.components.value(placement->componentId);
        if (positions.size() != definition.positions.size()) throw std::invalid_argument("Device LED count cannot change here");
        definition.positions = positions;
        // Copy-on-edit uses portable definitions without changing siblings or the saved project schema.
        const QString localId = "instance." + id.toString(QUuid::WithoutBraces);
        if (placement->componentId != localId && state.components.contains(localId))
            throw std::invalid_argument("Device geometry ID already exists");
        if (!state.components.contains(localId) && state.components.size() >= 256)
            throw std::invalid_argument("Catalog limit: 256 definitions");
        definition.id = localId;
        definition = ComponentDefinition::fromJson(definition.toJson());
        state.components.insert(localId, definition);
        placement->componentId = localId;
    });
}

void LightingDocument::importComponent(const QString& fileName)
{
    defineComponent(loadComponentDefinition(fileName));
}

QUuid LightingDocument::addComponent(const QString& componentId)
{
    ComponentPlacement placement;
    placement.componentId = componentId;
    edit("Add component", [&](LightingState& state) { state.placements.append(placement); });
    return placement.id;
}

QString LightingDocument::componentName(const ComponentPlacement& placement) const
{
    const auto name=hardwareName(catalog()[placement.componentId]);int count=0,ordinal=0;
    // Instance numbers describe identical models across the layout, never electrical chain addresses.
    for(const auto& candidate:state_.placements)if(hardwareName(catalog()[candidate.componentId]).compare(name,Qt::CaseInsensitive)==0) {
        ++count;if(candidate.id==placement.id)ordinal=count;
    }
    return count>1 ? name+" #"+QString::number(ordinal) : name;
}

QVector<QPointF> LightingDocument::componentPoints(const ComponentPlacement& placement) const
{
    const auto definition = catalog().constFind(placement.componentId);
    if (definition == catalog().cend()) throw std::invalid_argument("Unknown component");
    return placedPoints(placement,*definition);
}

QRectF LightingDocument::layoutBounds(const QVector<ComponentPlacement>* placements) const
{
    QRectF bounds(QPointF(),canvasSize());
    for(const auto& placement:placements ? *placements : state_.placements) {
        const auto points=componentPoints(placement);const auto& definition=catalog()[placement.componentId];
        if(definition.mapping.size()!=points.size())throw std::invalid_argument("Component geometry/mapping length mismatch");
        for(int i=0;i<points.size();++i) if(!definition.nonPhysicalSlots.contains(definition.mapping[i])) {
            // Retain a nonzero rectangle even for legacy coordinates whose magnitude exceeds pixel precision.
            const double margin=std::max(12.0,std::max(std::abs(points[i].x()),std::abs(points[i].y()))*1e-12);
            bounds=bounds.united(QRectF(points[i]-QPointF(margin,margin),QSizeF(2*margin,2*margin)));
        }
    }
    return bounds;
}

QMap<QUuid, QVector<QColor>> LightingDocument::sampleComponents(const QImage& frame, const QVector<ComponentPlacement>* placements) const
{
    if (frame.size() != canvasSize()) throw std::invalid_argument("Sampling requires a canvas-sized frame");
    // Premultiplication composites transparency onto black without boosting partially masked LEDs.
    const auto rgb = frame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    const auto bounds=layoutBounds(placements);
    QMap<QUuid, QVector<QColor>> result;
    for (const auto& placement : placements ? *placements : state_.placements) {
        result.insert(placement.id, sampleComponent(placement, rgb, bounds));
    }
    return result;
}

QVector<QColor> LightingDocument::sampleComponent(const ComponentPlacement& placement, const QImage& rgb, const QRectF& bounds) const
{
    const auto points = componentPoints(placement);
    if (catalog()[placement.componentId].mapping.size() != points.size())
        throw std::invalid_argument("Component geometry/mapping length mismatch");
    const auto mapping = channelMapping(ChannelProfile{"sampling", 65535}, {placement.componentId}, catalog());
    QVector<QColor> colors(points.size(), QColor(Qt::black));
    for (qsizetype i = 0; i < points.size(); ++i) {
        const QPointF point((points[i].x()-bounds.left())/bounds.width()*rgb.width(),
                            (points[i].y()-bounds.top())/bounds.height()*rgb.height());
        // Preview and hardware use the same world-to-raster mapping, including transient drags.
        if (point.x() >= 0 && point.y() >= 0 && point.x() < rgb.width() && point.y() < rgb.height()) {
            const auto pixel = rgb.pixel(int(point.x()), int(point.y()));
            colors[mapping[i]] = QColor(qRed(pixel), qGreen(pixel), qBlue(pixel));
        }
    }
    return colors;
}

void LightingDocument::assignChannel(const ChannelAssignment& assignment)
{
    edit("Configure channel", [&](LightingState& state) {
        if (!assignment.software) for (auto& layer : state.layers) layer.channelTargets.removeAll(assignment.id);
        for (auto& existing : state.assignments) if (existing.id == assignment.id) { existing = assignment; return; }
        state.assignments.append(assignment);
    });
}

QMap<QUuid, QVector<QColor>> LightingDocument::sampleChannels(const QImage& frame, double seconds, const QVector<ComponentPlacement>* placements) const
{
    if (!std::isfinite(seconds) || seconds<0 || seconds>86400) throw std::invalid_argument("Invalid global channel time");
    if (frame.size() != canvasSize()) throw std::invalid_argument("Sampling requires a canvas-sized frame");
    validateChannelAssignments(state_, profiles_);
    const auto rgb = frame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    const auto bounds=layoutBounds(placements);
    QMap<QUuid, QVector<QColor>> result;
    const bool restricted=std::any_of(state_.layers.cbegin(),state_.layers.cend(),[](const auto& layer) { return !layer.allChannels; });
    for (const auto& assignment : state_.assignments) if (assignment.software) {
        const auto raster=(restricted || assignment.timeScale!=1 || assignment.timeOffset!=0)
            ? renderForChannel(assignment.id,seconds) : rgb;
        QVector<QColor> chain;
        for (const auto& id : assignment.placements)
            for (const auto& placement : placements ? *placements : state_.placements) if (placement.id == id)
                chain.append(sampleComponent(placement, raster, bounds));
        result.insert(assignment.id, chain);
    }
    return result; // Native channels are deliberately absent, never overwritten with sampled colors.
}

double LightingDocument::channelTime(const QUuid& id, double seconds) const
{
    if (!std::isfinite(seconds) || seconds<0 || seconds>86400) throw std::invalid_argument("Invalid global channel time");
    for (const auto& assignment : state_.assignments) if (assignment.id==id && assignment.software)
        return std::clamp(seconds*assignment.timeScale+assignment.timeOffset,0.0,86400.0);
    throw std::invalid_argument("Unknown software channel");
}
QImage LightingDocument::renderForChannel(const QUuid& id, double seconds) const
{
    return render(state_,channelTime(id,seconds),id);
}

QImage LightingDocument::render(double seconds) const { return render(state_, seconds); }

void LightingDocument::save(const QString& fileName, bool markClean)
{
    QJsonArray layers, placements, components, assignments;
    validateChannelAssignments(state_, profiles_);
    for (const auto& assignment : state_.assignments) assignments.append(channelAssignmentJson(assignment));
    for (const auto& component : catalog()) components.append(component.toJson());
    for (const auto& layer : state_.layers) {
        QByteArray png;
        QBuffer buffer(&png); buffer.open(QIODevice::WriteOnly);
        if (!layer.mask.save(&buffer, "PNG")) throw std::runtime_error("Could not encode layer mask");
        QByteArray imagePng;
        if (!layer.image.isNull()) {
            QBuffer imageBuffer(&imagePng); imageBuffer.open(QIODevice::WriteOnly);
            if (!layer.image.save(&imageBuffer, "PNG")) throw std::runtime_error("Could not encode image");
        }
        QJsonObject advanced;
        for (const auto& parameter : advancedParameters()) advanced.insert(parameter.id, layer.*(parameter.member));
        QJsonArray cycle;
        for (const auto& color : layer.colorCycle) cycle.append(color.name(QColor::HexArgb));
        advanced.insert("colorCycle", cycle); advanced.insert("maskLinked", layer.maskLinked);
        advanced.insert("crop", QJsonArray{layer.crop.x(), layer.crop.y(), layer.crop.width(), layer.crop.height()});
        QJsonArray positions; for(double position:layer.gradientPositions) positions.append(position);
        QJsonArray gradient; for(const auto& color:layer.gradientColors) gradient.append(color.name(QColor::HexArgb));
        QJsonArray targets; for (const auto& id : layer.channelTargets) targets.append(id.toString());
        layers.append(QJsonObject{{"id", layer.id.toString()},
            {"gradientPositions",positions}, {"gradientColors", gradient}, {"targets", layer.allChannels ? QJsonValue(QJsonValue::Null) : QJsonValue(targets)}, {"name", layer.name},
            {"color", layer.color.name(QColor::HexArgb)}, {"visible", layer.visible},
            {"transparency", layer.transparency}, {"blend", static_cast<int>(layer.blend)},
            {"effect", QJsonObject{{"type", effectDefinition(layer.effect).id},
                {"secondaryColor", layer.secondaryColor.name(QColor::HexArgb)},
                {"centerX", layer.center.x()}, {"centerY", layer.center.y()},
                {"width", layer.extent.width()}, {"height", layer.extent.height()},
                {"rotation", layer.rotation}, {"repetitions", layer.repetitions}}},
            {"advanced", advanced},
            {"animation", QJsonObject{{"spin", layer.spin}, {"pulse", layer.pulse}, {"hueShift", layer.hueShift}}},
            {"imagePng", QString::fromLatin1(imagePng.toBase64())}, {"imageFill", layer.imageFill},
            {"maskEnabled", layer.maskEnabled}, {"maskPng", QString::fromLatin1(png.toBase64())}});
        if(layer.gradientFill) { auto json=layers.last().toObject(); json["gradientFill"]=QJsonArray{layer.gradientStart.x(),layer.gradientStart.y(),layer.gradientEnd.x(),layer.gradientEnd.y()}; layers[layers.size()-1]=json; }
    }
    for (const auto& placement : state_.placements)
        placements.append(QJsonObject{{"id", placement.id.toString()}, {"componentId", placement.componentId},
            {"x", placement.origin.x()}, {"y", placement.origin.y()},
            {"scaleX", placement.scale.width()}, {"scaleY", placement.scale.height()}, {"rotation", placement.rotation}});
    // Version 12 adds independent shape gradient fills; old readers must reject them rather than discard their appearance.
    const auto bytes = QJsonDocument(QJsonObject{{"schemaVersion", 12}, {"width", 640}, {"height", 400},
        {"layers", layers}, {"placements", placements}, {"components", components}, {"assignments", assignments}}).toJson();
    if (bytes.size() > 32 * 1024 * 1024) throw std::invalid_argument("Document exceeds 32 MiB; reduce images before saving");
    QSaveFile file(fileName);
    // Atomic replacement preserves an existing project if writing/commit fails.
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit())
        throw std::runtime_error("Could not save lighting document");
    if (markClean) history_.setClean();
}

void LightingDocument::load(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly) || file.size() > 32 * 1024 * 1024)
        throw std::invalid_argument("Document unreadable or larger than 32 MiB");
    QJsonParseError error;
    const auto parsed = QJsonDocument::fromJson(file.read(32 * 1024 * 1024 + 1), &error);
    const auto root = parsed.object();
    const double schema = root.value("schemaVersion").toDouble(-1);
    if (error.error != QJsonParseError::NoError || !parsed.isObject() ||
        (schema != 1 && schema != 2 && schema != 3 && schema != 4 && schema != 5 && schema != 6 && schema != 7 && schema != 8 && schema != 9 && schema != 10 && schema != 11 && schema != 12) || root.value("width").toDouble(-1) != 640 ||
        root.value("height").toDouble(-1) != 400 || !root.value("layers").isArray() || !root.value("placements").isArray())
        throw std::invalid_argument("Invalid lighting document header");
    const auto layerArray = root.value("layers").toArray();
    const auto placementArray = root.value("placements").toArray();
    if (layerArray.size() > 64 || placementArray.size() > 256)
        throw std::invalid_argument("Document exceeds layer/component limits");
    LightingState candidate;
    candidate.components = initialCatalog_;
    if (schema >= 7) {
        if (!root.value("components").isArray() || root.value("components").toArray().size() > 256)
            throw std::invalid_argument("Invalid embedded component catalog");
        candidate.components.clear();
        for (const auto& entry : root.value("components").toArray()) {
            if (!entry.isObject()) throw std::invalid_argument("Invalid embedded component");
            auto component = ComponentDefinition::fromJson(entry.toObject());
            // Repair the shipped RAM pixel/unit mistake only; preserve custom geometry and wire order.
            if (component.id == "ram.xpg" && component.positions.size() == 8 &&
                component.mapping == QVector<int>({0, 1, 2, 3, 4, 5, 6, 7})) {
                bool legacy = true;
                for (int i = 0; i < 8; ++i)
                    legacy = legacy && component.positions[i] == QPointF(i * 18, 0);
                if (legacy) for (auto& point : component.positions) point /= 20;
            }
            if (candidate.components.contains(component.id)) throw std::invalid_argument("Duplicate embedded component ID");
            candidate.components.insert(component.id, component);
        }
    } else if (root.contains("components")) {
        throw std::invalid_argument("Embedded components require schema 7");
    }
    qint64 imagePixels = 0;
    for (const auto& entry : layerArray) {
        const auto json = entry.toObject();
        const double transparency = json.value("transparency").toDouble(-1);
        const double blend = json.value("blend").toDouble(-1);
        if (!json.value("visible").isBool() || !json.value("maskEnabled").isBool() ||
            transparency < 0 || transparency > 100 || std::floor(transparency) != transparency ||
            blend < 0 || blend > 3 || std::floor(blend) != blend)
            throw std::invalid_argument("Invalid layer properties");
        EffectLayer layer;
        if(schema>=11) {
            const auto positions=json.value("gradientPositions");
            if(!positions.isArray() || positions.toArray().size()>16) throw std::invalid_argument("Invalid gradient positions");
            for(const auto& position:positions.toArray()) {
                if(!position.isDouble()) throw std::invalid_argument("Invalid gradient position");
                layer.gradientPositions.append(position.toDouble());
            }
        } else if(json.contains("gradientPositions")) throw std::invalid_argument("Gradient positions require schema 11");
        if(schema>=10) {
            const auto stops=json.value("gradientColors");
            if(!stops.isArray() || stops.toArray().size()>16) throw std::invalid_argument("Invalid gradient colors");
            for(const auto& value:stops.toArray()) {
                if(!value.isString()) throw std::invalid_argument("Gradient colors must be color strings");
                layer.gradientColors.append(QColor(value.toString()));
            }
        } else if(json.contains("gradientColors")) throw std::invalid_argument("Gradient colors require schema 10");
        if (schema >= 9) {
            const auto targets=json.value("targets");
            if (targets.isNull()) layer.allChannels=true;
            else if (targets.isArray() && targets.toArray().size()<=256) {
                layer.allChannels=false;
                for (const auto& value:targets.toArray()) {
                    if (!value.isString()) throw std::invalid_argument("Invalid layer target ID");
                    layer.channelTargets.append(QUuid(value.toString()));
                }
            } else throw std::invalid_argument("Invalid layer targets");
        } else if (json.contains("targets")) throw std::invalid_argument("Layer targets require schema 9");
        layer.id = QUuid(json.value("id").toString()); layer.name = json.value("name").toString();
        layer.color = QColor(json.value("color").toString()); layer.visible = json.value("visible").toBool();
        layer.transparency = static_cast<int>(transparency); layer.blend = static_cast<LayerBlend>(static_cast<int>(blend));
        layer.maskEnabled = json.value("maskEnabled").toBool();
        if (schema >= 2) {
            const auto effect = json.value("effect").toObject();
            for (const auto& key : {"centerX", "centerY", "width", "height", "rotation", "repetitions"})
                if (!effect.value(key).isDouble()) throw std::invalid_argument("Missing numeric effect parameter");
            layer.effect = effectDefinition(effect.value("type").toString()).kind;
            layer.secondaryColor = QColor(effect.value("secondaryColor").toString());
            layer.center = {effect.value("centerX").toDouble(), effect.value("centerY").toDouble()};
            layer.extent = {effect.value("width").toDouble(), effect.value("height").toDouble()};
            layer.rotation = effect.value("rotation").toDouble();
            const double repetitions = effect.value("repetitions").toDouble();
            if (repetitions < 1 || repetitions > 64 || std::floor(repetitions) != repetitions)
                throw std::invalid_argument("Invalid effect repetition count");
            layer.repetitions = static_cast<int>(repetitions);
            validateEffect(layer);
        } else if (json.contains("effect")) {
            throw std::invalid_argument("Effect parameters require document schema 2");
        }
        if(json.contains("gradientFill")) {
            if(schema<12 || !json.value("gradientFill").isArray() || json.value("gradientFill").toArray().size()!=4) throw std::invalid_argument("Shape gradient fills require schema 12 and four coordinates");
            const auto points=json.value("gradientFill").toArray();
            for(const auto& value:points) if(!value.isDouble()) throw std::invalid_argument("Invalid gradient fill coordinate");
            layer.gradientFill=true; layer.gradientStart={points[0].toDouble(),points[1].toDouble()}; layer.gradientEnd={points[2].toDouble(),points[3].toDouble()};
            validateEffect(layer);
        }
        if(schema<11 && layer.effect==EffectKind::Paint) throw std::invalid_argument("Paint layers require schema 11");
        if (schema < 3 && (layer.effect == EffectKind::Image || json.contains("imagePng")))
            throw std::invalid_argument("Images require document schema 3");
        if (schema >= 3) {
            if (!json.value("imageFill").isBool() || !json.value("imagePng").isString())
                throw std::invalid_argument("Invalid image properties");
            layer.imageFill = json.value("imageFill").toBool();
            const auto imageEncoded = json.value("imagePng").toString().toLatin1();
            if (!imageEncoded.isEmpty()) {
                const auto data = QByteArray::fromBase64Encoding(imageEncoded, QByteArray::AbortOnBase64DecodingErrors);
                if (!data) throw std::invalid_argument("Invalid encoded image");
                QBuffer imageBuffer; imageBuffer.setData(data.decoded); imageBuffer.open(QIODevice::ReadOnly);
                QImageReader imageReader(&imageBuffer, "PNG");
                const auto size = imageReader.size();
                if (!size.isValid() || size.width() > 4096 || size.height() > 4096)
                    throw std::invalid_argument("Invalid image dimensions");
                imagePixels += qint64(size.width()) * size.height();
                if (imagePixels > 16 * 1024 * 1024)
                    throw std::invalid_argument("Document image budget exceeds 16 megapixels");
                layer.image = imageReader.read().convertToFormat(QImage::Format_ARGB32_Premultiplied);
                if (layer.image.isNull()) throw std::invalid_argument("Invalid image pixels");
            }
        }
        if (schema >= 4) {
            const auto animation = json.value("animation").toObject();
            for (const auto& key : {"spin", "pulse", "hueShift"})
                if (!animation.value(key).isDouble()) throw std::invalid_argument("Invalid animation rates");
            layer.spin = animation.value("spin").toDouble();
            layer.pulse = animation.value("pulse").toDouble();
            layer.hueShift = animation.value("hueShift").toDouble();
        } else if (json.contains("animation")) {
            throw std::invalid_argument("Animation requires document schema 4");
        }
        if (schema >= 5) {
            const auto advanced = json.value("advanced").toObject();
            for (const auto& parameter : advancedParameters()) {
                if (!advanced.value(parameter.id).isDouble()) throw std::invalid_argument("Missing advanced parameter");
                layer.*(parameter.member) = advanced.value(parameter.id).toDouble();
            }
            if (!advanced.value("maskLinked").isBool() || !advanced.value("colorCycle").isArray())
                throw std::invalid_argument("Invalid linked mask/color cycle");
            layer.maskLinked = advanced.value("maskLinked").toBool(); layer.colorCycle.clear();
            const auto cycle = advanced.value("colorCycle").toArray();
            if (cycle.size() < 2 || cycle.size() > 16) throw std::invalid_argument("Invalid color cycle size");
            for (const auto& color : cycle) layer.colorCycle.append(QColor(color.toString()));
            const auto crop = advanced.value("crop").toArray();
            if (crop.size() != 4) throw std::invalid_argument("Invalid image crop");
            for (const auto& value : crop) if (!value.isDouble()) throw std::invalid_argument("Invalid crop coordinate");
            layer.crop = {crop[0].toDouble(),crop[1].toDouble(),crop[2].toDouble(),crop[3].toDouble()};
        } else if (json.contains("advanced") || isParticle(layer.effect)) {
            throw std::invalid_argument("Advanced effects require schema 5");
        }
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
        if (schema >= 6) {
            if (!json.value("scaleX").isDouble() || !json.value("scaleY").isDouble() || !json.value("rotation").isDouble())
                throw std::invalid_argument("Invalid placement transform");
            placement.scale = {json.value("scaleX").toDouble(), json.value("scaleY").toDouble()};
            placement.rotation = json.value("rotation").toDouble();
        } else if (json.contains("scaleX") || json.contains("scaleY") || json.contains("rotation")) {
            // Older readers must not silently discard geometry written under the wrong schema.
            throw std::invalid_argument("Placement transforms require schema 6");
        }
        if (!candidate.components.contains(placement.componentId)) throw std::invalid_argument("Document requires an unavailable component");
        candidate.placements.append(placement);
    }
    if (schema >= 8) {
        if (!root.value("assignments").isArray() || root.value("assignments").toArray().size() > 256)
            throw std::invalid_argument("Invalid channel assignment array");
        for (const auto& entry : root.value("assignments").toArray()) {
            if (!entry.isObject()) throw std::invalid_argument("Invalid channel assignment");
            candidate.assignments.append(parseChannelAssignment(entry.toObject(),int(schema)));
        }
    } else if (root.contains("assignments")) {
        throw std::invalid_argument("Channel assignments require schema 8");
    }
    validate(candidate);
    validateChannelAssignments(candidate, profiles_);
    // Parse/validate everything before replacing the live document and undo history.
    state_ = std::move(candidate);
    history_.clear();
    history_.setClean();
    ++revision_; emit changed();
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

QImage LightingDocument::render(const LightingState& state, double seconds, const QUuid& channel)
{
    validate(state);
    if (!std::isfinite(seconds) || seconds < 0 || seconds > 86400)
        throw std::invalid_argument("Invalid preview time");
    QImage result(canvasSize(), QImage::Format_ARGB32_Premultiplied);
    result.fill(Qt::transparent);
    QPainter painter(&result);
    for (const auto& layer : state.layers) {
        if (!layer.visible || layer.transparency == 100 ||
            (!channel.isNull() && !layer.allChannels && !layer.channelTargets.contains(channel))) continue;
        auto source = renderEffect(layer, seconds);
        const auto maskImage = renderMask(layer,seconds);
        for (int y = 0; y < source.height(); ++y) {
            auto* pixels = reinterpret_cast<QRgb*>(source.scanLine(y));
            const auto* mask = maskImage.constScanLine(y);
            for (int x = 0; x < source.width(); ++x) {
                const int coverage = layer.maskEnabled ? mask[x] : 255;
                const double alphaScale = (100 - layer.transparency) / 100.0 * coverage / 255.0;
                const auto pixel = pixels[x];
                pixels[x] = qRgba(qRound(qRed(pixel) * alphaScale), qRound(qGreen(pixel) * alphaScale),
                    qRound(qBlue(pixel) * alphaScale), qRound(qAlpha(pixel) * alphaScale));
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
