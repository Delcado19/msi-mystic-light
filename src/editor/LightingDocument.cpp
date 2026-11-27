#include "LightingDocument.h"
#include "components/ComponentLibrary.h"
#include "HardwareIdentity.h"
#include "HardwareArtwork.h"
#include <QPainter>
#include <QTransform>
#include <QUndoCommand>
#include <QSet>
#include <QBuffer>
#include <QFile>
#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QLockFile>
#include <QRegularExpression>
#include <QSaveFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QImageReader>
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <limits>
#include <memory>

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

// The user library is a folder with one JSON file per component (plan 2026-11-27 Task 3). Unreadable
// files are skipped here; readComponentFolder() reports them so the UI can name them (Task 5).
ComponentCatalog readLibrary(const QString& folder, const ComponentCatalog& builtIn)
{
    if (folder.isEmpty() || !QFileInfo(folder).isDir()) return {};
    return readComponentFolder(folder, builtIn).components;
}

// Brings the folder in line with the catalog: changed or new entries are written, entries no longer in
// the catalog lose their file. Files that could not be read are never touched.
void writeLibrary(const QString& folder, const ComponentCatalog& components)
{
    const auto current = readComponentFolder(folder, {}).components;
    for (const auto& definition : components)
        if (!current.contains(definition.id) || current.value(definition.id).toJson() != definition.toJson())
            writeComponentFile(folder, definition);
    for (const auto& id : current.keys())
        if (!components.contains(id) && !removeComponentFile(folder, id))
            throw std::runtime_error(("Cannot delete the file of " + id).toStdString());
}

// QLockFile inside the library folder; creates the folder first.
QString libraryLock(const QString& folder)
{
    if (!QDir().mkpath(folder)) throw std::runtime_error("Cannot create custom component directory");
    return QDir(folder).filePath(".library.lock");
}

QString reconcileDefinition(ComponentDefinition definition, const ComponentCatalog& installed, ComponentCatalog& custom)
{
    auto same = [&](const ComponentCatalog& catalog) {
        return catalog.contains(definition.id) && catalog.value(definition.id).toJson() == definition.toJson();
    };
    if (same(installed) || same(custom)) return definition.id;
    if (installed.contains(definition.id) || custom.contains(definition.id)) {
        // A content-derived ID makes a second open of an old project reuse the same variant.
        const auto identity = QJsonDocument(definition.toJson()).toJson(QJsonDocument::Compact);
        definition.id += "." + QString::fromLatin1(QCryptographicHash::hash(identity, QCryptographicHash::Sha256).toHex());
        if (same(installed) || same(custom)) return definition.id;
        if (installed.contains(definition.id) || custom.contains(definition.id))
            throw std::invalid_argument("Conflicting component variant ID");
    }
    custom.insert(definition.id, definition);
    return definition.id;
}

QImage composeLighting(const QVector<EffectLayer>& layers, double seconds, const QUuid& channel,
                       const QVector<QPoint>* addresses = nullptr);

QColor identifyColor(const QColor& color)
{
    // A white baseline needs a dark flash to make Identify visible.
    return color.red()>220 && color.green()>220 && color.blue()>220 ? Qt::black : Qt::white;
}

void validate(const LightingState& state)
{
    if (state.layers.size() > 64 || state.placements.size() > 256)
        throw std::invalid_argument("Document limit: 64 effect layers and 256 component placements");
    qint64 imagePixels = 0;
    QSet<QUuid> ids;
    for (const auto& layer : state.layers) {
        validateEffect(layer);
        if(layer.container && (layer.effect!=EffectKind::Paint || !layer.image.isNull()))throw std::invalid_argument("Layer containers cannot replace their source");
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
    for(const auto& layer:state.layers)if(!layer.parentId.isNull()) {
        const auto parent=std::find_if(state.layers.cbegin(),state.layers.cend(),[&](const auto& item){return item.id==layer.parentId;});
        if(layer.container || parent==state.layers.cend() || !parent->parentId.isNull() || parent->id==layer.id)
            throw std::invalid_argument("Object requires an existing root layer");
    }
    for (const auto& placement : state.placements) {
        if (placement.id.isNull() || ids.contains(placement.id) || !state.components.contains(placement.componentId))
            throw std::invalid_argument("Invalid component placement");
        placedPoints(placement,state.components[placement.componentId]);
        ids.insert(placement.id);
    }
}
}

MissingComponents::MissingComponents(QStringList missing)
    : std::invalid_argument(("Missing components: " + missing.join(", ")).toStdString()), ids(std::move(missing)) {}

LightingDocument::LightingDocument(ComponentCatalog catalog, QObject* parent, QVector<DeviceProfile> profiles, QString libraryPath)
    : QObject(parent), profiles_(std::move(profiles)), initialCatalog_(std::move(catalog)), libraryPath_(std::move(libraryPath))
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
    if(colorPreviewBefore_){state_=std::move(after);++revision_;emit changed();return;}
    history_.push(new DocumentEdit(label, [this, before = state_, after](bool forward) {
        state_ = forward ? after : before;
        ++revision_; emit changed();
    }));
}

void LightingDocument::beginColorPreview()
{
    if(colorPreviewBefore_)throw std::logic_error("A color preview is already active");
    colorPreviewBefore_=state_;
}
void LightingDocument::finishColorPreview(bool accept)
{
    if(!colorPreviewBefore_)return;
    const auto after=state_;state_=std::move(*colorPreviewBefore_);colorPreviewBefore_.reset();
    if(accept)edit("Color edit",[&](LightingState& state){state=after;});
    else {++revision_;emit changed();}
}

QImage LightingDocument::whiteMask()
{
    QImage mask(canvasSize(), QImage::Format_Grayscale8);
    mask.fill(255);
    return mask;
}

static EffectLayer makeSource(EffectKind effect,const QUuid& parent={})
{
    EffectLayer layer;
    layer.parentId=parent;
    layer.effect = effect;
    const auto& definition=effectDefinition(effect);
    layer.name = definition.label;layer.patternSpeed=definition.patternSpeed;layer.particleCount=definition.particleCount;layer.patternSites=definition.patternSites;
    if (isGradient(effect) || effect==EffectKind::Fireworks || effect==EffectKind::Lightning || effect==EffectKind::Starfield) layer.extent = LightingDocument::canvasSize();
    if(isPattern(effect)) { layer.gradientColors={QColor("#42d9ff"),QColor("#c65cff"),QColor("#050810")}; layer.secondaryColor=layer.gradientColors.last(); }
    if(effect==EffectKind::Aurora) layer.gradientColors={QColor("#30ffc2"),QColor("#715cff"),QColor("#020515")};
    if(effect==EffectKind::LavaLamp) layer.gradientColors={QColor("#100822"),QColor("#d83980"),QColor("#ffc474")};
    if(effect==EffectKind::Checkerboard) layer.gradientColors={QColor("#080a0e"),QColor("#dce7f0")};
    if(effect==EffectKind::Fireplace) {layer.gradientColors={QColor("#0d0200"),QColor("#dc2305"),QColor("#ffac20"),QColor("#fff1ad")};}
    if(effect==EffectKind::Radar) {layer.extent={400,400};layer.gradientColors={QColor("#75ff95"),QColor("#139347"),QColor("#020d06")};}
    if(effect==EffectKind::Burst) {layer.color=QColor("#ffe078");layer.secondaryColor=QColor("#ff527f");}
    if(effect==EffectKind::Lightning) {layer.color=QColor("#7e9dff");layer.secondaryColor=Qt::white;}
    if(effect==EffectKind::Starfield) {layer.color=QColor("#e5f0ff");layer.secondaryColor=QColor("#6b8fd9");}
    if(effect==EffectKind::Fireworks) {layer.color=QColor("#ffd36b");layer.secondaryColor=QColor("#ff4f70");}
    layer.mask = LightingDocument::whiteMask();
    return layer;
}
EffectLayer LightingDocument::effectDefaults(EffectKind effect) { return makeSource(effect); }

QUuid LightingDocument::addLayer(EffectKind effect, const QUuid& parent)
{
    const auto layer=makeSource(effect,parent);
    edit("Add effect layer", [&](LightingState& state) { state.layers.append(layer); });
    return layer.id;
}

QUuid LightingDocument::addMultizoneLayer()
{
    auto root=makeSource(EffectKind::Paint);root.container=true;root.name="Multizone";root.extent=canvasSize();
    const QVector<EffectKind> kinds{EffectKind::Pinwheel,EffectKind::Spiral,EffectKind::Voronoi,EffectKind::Fireplace};
    const QStringList names{"Top left","Top right","Bottom left","Bottom right"};
    edit("Add Multizone",[&](auto& state) {
        if(state.layers.size()>59) throw std::invalid_argument("Multizone needs five available layer/object slots");
        state.layers.append(root);
        for(int i=0;i<4;++i) {
            auto zone=makeSource(kinds[i],root.id);zone.name=names[i];
            const QRect bounds((i%2)*320,(i/2)*200,320,200);
            zone.center={bounds.x()+160.,bounds.y()+100.};zone.extent=bounds.size();
            // Each zone is an ordinary source with its own linked mask, not an opaque full-canvas overlay.
            zone.mask.fill(0);{QPainter painter(&zone.mask);painter.fillRect(bounds,Qt::white);}zone.maskLinked=true;
            state.layers.append(zone);
        }
    });
    return root.id;
}

QUuid LightingDocument::addPaintLayer(const QUuid& parent)
{
    EffectLayer layer; layer.parentId=parent; layer.effect=EffectKind::Paint; layer.name="Layer "+QString::number(state_.layers.size()+1);
    layer.extent=canvasSize(); layer.mask=whiteMask();
    // A null source is a transparent canvas until the first drawing gesture.
    edit("New layer",[&](LightingState& state) { state.layers.append(layer); });
    return layer.id;
}

QUuid LightingDocument::layerRoot(const QUuid& object) const
{
    for(const auto& layer:state_.layers)if(layer.id==object)return layer.parentId.isNull() ? layer.id : layer.parentId;
    return {};
}

QUuid LightingDocument::addEmptyLayer()
{
    EffectLayer layer;layer.container=true;layer.effect=EffectKind::Paint;layer.extent=canvasSize();layer.mask=whiteMask();
    layer.name="Group "+QString::number(state_.layers.size()+1);
    edit("New group",[&](auto& state){state.layers.append(layer);});return layer.id;
}

QUuid LightingDocument::creationParent(const QUuid& selection) const
{
    for(const auto& layer:state_.layers)if(layer.id==selection) {
        // Existing source parents keep ownership; new sources join their children as siblings.
        const bool ownsChildren=std::any_of(state_.layers.cbegin(),state_.layers.cend(),[&](const auto& child){return child.parentId==layer.id;});
        const auto parent=layer.container || ownsChildren ? layer.id : layer.parentId;
        for(const auto& root:state_.layers)if(root.id==parent)return parent;
    }
    return {};
}

QUuid LightingDocument::importImage(const QString& fileName, const QUuid& parent)
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
    EffectLayer layer;layer.parentId=parent;
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

void LightingDocument::setComponentGeometry(const QUuid& id, const QVector<QPointF>& positions,
                                             const std::optional<QVector<int>>& layers)
{
    if (!libraryPath_.isEmpty()) {
        for (const auto& placement : state_.placements) if (placement.id == id) {
            auto definition = state_.components.value(placement.componentId);
            if (definition.positions.size() != positions.size())
                throw std::invalid_argument("Device LED count cannot change here");
            definition.positions = positions;
            if (layers) definition.layers = *layers;
            applyComponentVariant(id, definition);
            return;
        }
        throw std::invalid_argument("Select a component first");
    }
    edit("Edit device LED arrangement", [&](LightingState& state) {
        auto placement = std::find_if(state.placements.begin(), state.placements.end(),
            [&](const auto& item) { return item.id == id; });
        if (placement == state.placements.end()) throw std::invalid_argument("Select a component first");
        auto definition = state.components.value(placement->componentId);
        if (positions.size() != definition.positions.size()) throw std::invalid_argument("Device LED count cannot change here");
        definition.positions = positions;
        if (layers) definition.layers = *layers;
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

ComponentCatalog LightingDocument::customComponents() const
{
    if (!libraryPath_.isEmpty()) return readLibrary(libraryPath_, initialCatalog_);
    auto custom = state_.components;
    for (const auto& definition : initialCatalog_) custom.remove(definition.id);
    return custom;
}

QString LightingDocument::registerComponent(const ComponentDefinition& definition)
{
    const auto validated = ComponentDefinition::fromJson(definition.toJson());
    if (libraryPath_.isEmpty()) {
        defineComponent(validated);
        return validated.id;
    }
    QLockFile lock(libraryLock(libraryPath_));
    if (!lock.tryLock()) throw std::runtime_error("Custom component library is busy");
    auto custom = readLibrary(libraryPath_, initialCatalog_);
    const auto id = reconcileDefinition(validated, initialCatalog_, custom);
    writeLibrary(libraryPath_, custom);
    return id;
}

void LightingDocument::importComponent(const QString& fileName)
{
    registerComponent(loadComponentDefinition(fileName));
}

void LightingDocument::applyComponentVariant(const QUuid& id, const ComponentDefinition& definition)
{
    const auto validated = ComponentDefinition::fromJson(definition.toJson());
    std::unique_ptr<QLockFile> lock;
    ComponentCatalog custom;
    if (!libraryPath_.isEmpty()) {
        lock=std::make_unique<QLockFile>(libraryLock(libraryPath_));
        if (!lock->tryLock()) throw std::runtime_error("Custom component library is busy");
        custom=readLibrary(libraryPath_, initialCatalog_);
    }
    edit("Apply component variant", [&](LightingState& state) {
        for (auto& placement : state.placements) if (placement.id == id) {
            if (placement.locked) throw std::invalid_argument("Unlock the component before editing");
            if (!lock) {
                placement.componentId=reconcileDefinition(validated,initialCatalog_,state.components);
                return;
            }
            const auto variantId=reconcileDefinition(validated,initialCatalog_,custom);
            state.components.insert(variantId,custom.value(variantId,initialCatalog_.value(variantId)));
            placement.componentId = variantId;
            // Reject over-capacity wiring before publishing the variant to the shared library.
            validate(state);
            validateChannelAssignments(state,profiles_);
            writeLibrary(libraryPath_,custom);
            lock.reset();
            return;
        }
        throw std::invalid_argument("Select a component first");
    });
}

QVector<QUuid> LightingDocument::componentUsers(const QString& id) const
{
    QVector<QUuid> users;
    for (const auto& placement : state_.placements) if (placement.componentId == id) users.append(placement.id);
    return users;
}

void LightingDocument::removeCustomComponent(const QString& id)
{
    if (initialCatalog_.contains(id)) throw std::invalid_argument("Built-in components cannot be deleted");
    // Deleting a definition the open layout still uses would leave placements without geometry.
    if (const auto users = componentUsers(id); !users.isEmpty()) {
        QStringList names;
        for (const auto& placement : state_.placements) if (users.contains(placement.id)) names.append(componentName(placement));
        throw std::invalid_argument(QString("%1 is used by %2. Remove it from the layout first.")
            .arg(hardwareName(catalog().value(id)), names.join(", ")).toStdString());
    }
    if (libraryPath_.isEmpty()) {
        if (!state_.components.contains(id)) throw std::invalid_argument("Unknown custom component");
        edit("Delete component", [&](LightingState& state) { state.components.remove(id); });
        return;
    }
    QLockFile lock(libraryLock(libraryPath_));
    if (!lock.tryLock()) throw std::runtime_error("Custom component library is busy");
    auto custom = readLibrary(libraryPath_, initialCatalog_);
    if (!custom.remove(id)) throw std::invalid_argument("Unknown custom component");
    writeLibrary(libraryPath_, custom);
    state_.components.remove(id);
}

void LightingDocument::updateCustomComponent(const ComponentDefinition& definition)
{
    // Deliberately bypasses reconcileDefinition(): saving an edited library entry must overwrite it,
    // not fork an "<id>.<sha256>" variant (the source of duplicate entries in owners' libraries).
    const auto validated = ComponentDefinition::fromJson(definition.toJson());
    if (initialCatalog_.contains(validated.id)) throw std::invalid_argument("Built-in components cannot be changed; save a copy");
    std::unique_ptr<QLockFile> lock;
    ComponentCatalog custom;
    if (!libraryPath_.isEmpty()) {
        lock = std::make_unique<QLockFile>(libraryLock(libraryPath_));
        if (!lock->tryLock()) throw std::runtime_error("Custom component library is busy");
        custom = readLibrary(libraryPath_, initialCatalog_);
        if (!custom.contains(validated.id)) throw std::invalid_argument("Unknown custom component");
    } else if (!state_.components.contains(validated.id)) throw std::invalid_argument("Unknown custom component");
    const bool used = state_.components.contains(validated.id);
    // edit() validates channel capacity before anything is written to the library.
    if (used) edit("Edit component", [&](LightingState& state) { state.components.insert(validated.id, validated); });
    if (!lock) return;
    custom.insert(validated.id, validated);
    try { writeLibrary(libraryPath_, custom); }
    catch (...) { if (used) history_.undo(); throw; }
}

QString LightingDocument::saveComponentCopy(const ComponentDefinition& definition)
{
    auto copy = definition;
    auto slug = definition.name.toLower();
    slug.replace(QRegularExpression("[^a-z0-9]+"), "-");
    slug = slug.mid(0, 40).remove(QRegularExpression("^-+|-+$"));
    copy.id = QString("custom.%1.%2").arg(slug.isEmpty() ? "component" : slug, QUuid::createUuid().toString(QUuid::Id128).left(8));
    return registerComponent(copy);
}

QUuid LightingDocument::duplicateComponent(const QUuid& id)
{
    const auto found = std::find_if(state_.placements.cbegin(), state_.placements.cend(),
        [&](const auto& placement) { return placement.id == id; });
    if (found == state_.placements.cend()) throw std::invalid_argument("Select a component first");
    auto copy = *found;
    copy.id = QUuid::createUuid();
    copy.origin += QPointF(24, 24);
    copy.locked = false;
    edit("Duplicate component", [&](LightingState& state) {
        state.placements.append(copy);
        for (auto& assignment : state.assignments) if (assignment.placements.contains(id)) {
            if (!chainStarts(assignment).isEmpty()) assignment.starts[copy.id]=outputLedsUsed(assignment)+1;
            assignment.placements.append(copy.id);
            break;
        }
    });
    return copy.id;
}

void LightingDocument::removeComponents(const QSet<QUuid>& ids)
{
    edit("Remove components", [&](LightingState& state) {
        for (auto& assignment : state.assignments) {
            for (const auto& id : ids) {
                assignment.placements.removeAll(id);
                assignment.starts.remove(id);
            }
        }
        QSet<QUuid> empty;
        for (const auto& assignment : state.assignments)
            if (assignment.software && assignment.placements.isEmpty()) empty.insert(assignment.id);
        state.assignments.removeIf([&](const auto& assignment) { return empty.contains(assignment.id); });
        for (auto& layer : state.layers) for (const auto& id : empty) layer.channelTargets.removeAll(id);
        state.placements.removeIf([&](const auto& placement) { return ids.contains(placement.id); });
    });
}

void LightingDocument::moveComponent(const QUuid& id, ChannelAssignment destination)
{
    const auto& channel = assignmentChannel(assignmentProfile(profiles_, destination.profileId), destination.channelId);
    if (!channel.direct || channelHasFixedComponent(channel))
        throw std::invalid_argument("Choose a connector that accepts components");
    if (destination.placements.contains(id)) return;
    edit("Move component to connector", [&](LightingState& state) {
        QSet<QUuid> empty;
        for (auto& assignment : state.assignments) if (assignment.placements.contains(id)) {
            const auto& source = assignmentChannel(assignmentProfile(profiles_, assignment.profileId), assignment.channelId);
            if (channelHasFixedComponent(source)) throw std::invalid_argument("Integrated components cannot move");
            assignment.placements.removeAll(id);
            assignment.starts.remove(id);
            if (assignment.software && assignment.placements.isEmpty()) empty.insert(assignment.id);
        }
        state.assignments.removeIf([&](const auto& assignment) { return empty.contains(assignment.id); });
        for (auto& layer : state.layers) for (const auto& assignment : empty) layer.channelTargets.removeAll(assignment);
        if (!chainStarts(destination).isEmpty()) destination.starts[id]=outputLedsUsed(destination)+1;
        destination.placements.append(id);
        for (auto& assignment : state.assignments) if (assignment.id==destination.id) {
            assignment = destination;
            return;
        }
        state.assignments.append(destination);
    });
}

int LightingDocument::availableAddresses(const ChannelAssignment& assignment) const
{
    const auto& profile = assignmentProfile(profiles_, assignment.profileId);
    const auto& channel = assignmentChannel(profile, assignment.channelId);
    const int capacity = channel.fixedLeds ? channel.fixedLeds : channel.maxLeds;
    return capacity - outputLedsUsed(assignment);
}

QUuid LightingDocument::addComponent(const QString& componentId)
{
    ComponentPlacement placement;
    placement.componentId = componentId;
    const auto definition = catalog().contains(componentId) ? catalog().value(componentId) : customComponents().value(componentId);
    edit("Add component", [&](LightingState& state) {
        if (!definition.id.isEmpty()) state.components.insert(componentId, definition);
        state.placements.append(placement);
    });
    return placement.id;
}

QString LightingDocument::componentName(const ComponentPlacement& placement) const
{
    // Layout asks for labels repeatedly per frame; normalize/count once per document revision, including undo/load.
    if(nameRevision_!=revision_) {
        componentNames_.clear();QMap<QString,int> counts,ordinals;QMap<QString,QString> names;
        for(const auto& candidate:state_.placements) {
            if(!names.contains(candidate.componentId))names[candidate.componentId]=hardwareName(catalog()[candidate.componentId]);
            ++counts[names[candidate.componentId].toCaseFolded()];
        }
        for(const auto& candidate:state_.placements) {
            auto name=names[candidate.componentId];const auto key=name.toCaseFolded();
            const int ordinal=++ordinals[key];
            if(counts[key]>1)name+=" #"+QString::number(ordinal);
            componentNames_[candidate.id]=name;
        }
        nameRevision_=revision_;
    }
    const auto found=componentNames_.constFind(placement.id);
    return found!=componentNames_.cend() ? *found : hardwareName(catalog()[placement.componentId]);
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
        // Housings are part of the component too; they must not stick out of the effect area.
        QTransform body;body.translate(placement.origin.x(),placement.origin.y());body.rotate(placement.rotation);
        body.scale(20*placement.scale.width(),20*placement.scale.height());
        const auto housing=body.mapRect(hardwareBodyBounds(definition));
        if(housing.isValid() && std::isfinite(housing.width()) && std::isfinite(housing.height()))bounds=bounds.united(housing);
    }
    // Keep the raster's 640:400 aspect: growing in one direction pads the other instead of
    // stretching or squashing the effect (owner report 2026-11-14).
    const double aspect=640.0/400.0;
    if(bounds.width()>bounds.height()*aspect){const double pad=(bounds.width()/aspect-bounds.height())/2;bounds.adjust(0,-pad,0,pad);}
    else if(bounds.width()<bounds.height()*aspect){const double pad=(bounds.height()*aspect-bounds.width())/2;bounds.adjust(-pad,0,pad,0);}
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
    const auto count = catalog()[placement.componentId].addresses();
    QVector<QColor> colors(count, QColor(Qt::black));
    QVector<bool> sampled(count, false);
    for (qsizetype i = 0; i < points.size(); ++i) {
        // A zone takes its color from its first declared light point, regardless of how many points mirror it.
        if (sampled[mapping[i]]) continue;
        sampled[mapping[i]] = true;
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
    const bool chase = std::any_of(state_.layers.cbegin(),state_.layers.cend(),[](const auto& layer) {
        return layer.effect == EffectKind::Chase && layer.visible;
    });
    for (const auto& assignment : state_.assignments) if (assignment.software) {
        const auto raster=(restricted || assignment.timeScale!=1 || assignment.timeOffset!=0)
            ? renderForChannel(assignment.id,seconds) : rgb;
        QVector<QVector<QColor>> sampled;
        QVector<int> counts;
        QVector<QUuid> ids;
        for (const auto& id : assignment.placements)
            for (const auto& placement : placements ? *placements : state_.placements) if (placement.id == id) {
                ids.append(id); sampled.append(sampleComponent(placement, raster, bounds)); counts.append(int(sampled.last().size()));
            }
        // A preview subset may omit placements; positions then fall back to the serial order.
        const auto starts = counts.size() == assignment.placements.size() ? chainStarts(assignment) : QVector<int>{};
        const auto offsets = chainOffsets(counts, starts);
        QVector<QColor> chain;
        QVector<bool> owned;
        QVector<QPoint> addressPoints;
        // Shared output positions have one physical color: the earlier chain item supplies it,
        // later components mirror it. Unaddressed gaps stay black.
        for (qsizetype index = 0; index < sampled.size(); ++index) {
            const auto end = offsets[index] + counts[index];
            if (chain.size() < end) {
                chain.resize(end, QColor(Qt::black));
                owned.resize(end, false);
                if (chase) addressPoints.resize(end, QPoint(-1,-1));
            }
            if (chase) {
                const auto& instances=placements ? *placements : state_.placements;
                const auto placement=std::find_if(instances.cbegin(),instances.cend(),[&](const auto& item) { return item.id==ids[index]; });
                const auto points=componentPoints(*placement);
                const auto& mapping=catalog()[placement->componentId].mapping;
                QVector<bool> supplied(counts[index],false);
                for (int i=0;i<points.size();++i) {
                    const auto address=mapping.at(i);
                    const int output=offsets[index]+address;
                    if (owned[output] || supplied[address]) continue;
                    supplied[address]=true;
                    const double x=(points[i].x()-bounds.left())/bounds.width()*rgb.width();
                    const double y=(points[i].y()-bounds.top())/bounds.height()*rgb.height();
                    if (x>=0 && x<rgb.width() && y>=0 && y<rgb.height()) addressPoints[output]=QPoint(int(x),int(y));
                }
            }
            for (int led = 0; led < counts[index]; ++led) if (!owned[offsets[index] + led]) {
                chain[offsets[index] + led] = sampled[index][led]; owned[offsets[index] + led] = true;
            }
        }
        if (chase && !addressPoints.isEmpty()) {
            const auto composed=composeLighting(state_.layers,channelTime(assignment.id,seconds),assignment.id,&addressPoints);
            for (int address=0;address<chain.size();++address) {
                const auto pixel=composed.pixel(address,0);
                chain[address]=QColor(qRed(pixel),qGreen(pixel),qBlue(pixel));
            }
        }
        // Overlay shared addresses after composing the frame so the rest keeps playing.
        if (!identify_.isNull()) {
            const auto at = ids.indexOf(identify_);
            if (at >= 0) for (int led = 0; led < counts[at]; ++led) {
                auto& color=chain[offsets[at]+led];
                color=identifyColor(color);
            }
        }
        result.insert(assignment.id, chain);
    }
    return result; // Native channels are deliberately absent, never overwritten with sampled colors.
}

QMap<QUuid, QVector<QColor>> LightingDocument::previewComponentColors(const QImage& frame, double seconds, const QVector<ComponentPlacement>* placements, const QImage* channelFrame) const
{
    auto sampled = sampleComponents(frame, placements);
    // Output colors come from the global composition, not from one selected channel's raster
    // (CodeRabbit review 2026-11-15); clocked channels re-render themselves in sampleChannels.
    const auto channels = sampleChannels(channelFrame ? *channelFrame : frame, seconds, placements);
    for (const auto& assignment : state_.assignments) {
        if (!assignment.software) {
            QColor color = Qt::black;
            if (assignment.native.id != "off" && !assignment.native.colors.isEmpty()) {
                const auto rgb = assignment.native.colors.first(); color = QColor(rgb.r, rgb.g, rgb.b);
            }
            for (const auto& id : assignment.placements) if (sampled.contains(id)) sampled[id].fill(color);
            continue;
        }
        // Same offsets as sampleChannels; a preview subset falls back to serial order there too.
        QVector<int> counts;
        for (const auto& id : assignment.placements) if (sampled.contains(id)) counts.append(int(sampled[id].size()));
        const bool complete = counts.size() == assignment.placements.size();
        const auto offsets = chainOffsets(counts, complete ? chainStarts(assignment) : QVector<int>{});
        const auto colors = channels.value(assignment.id);
        int index = 0;
        for (const auto& id : assignment.placements) if (sampled.contains(id)) {
            sampled[id] = colors.mid(offsets[index], counts[index]); ++index;
        }
    }
    if (!identify_.isNull() && sampled.contains(identify_) && !hasSoftwareChannel(identify_))
        for (auto& color:sampled[identify_]) color=identifyColor(color);
    return sampled;
}

QMap<QUuid, int> LightingDocument::componentLedOffsets() const
{
    QMap<QUuid, int> result;
    for (const auto& assignment : state_.assignments) if (assignment.software) {
        QVector<int> counts;
        for (const auto& id : assignment.placements)
            for (const auto& placement : state_.placements) if (placement.id == id)
                counts.append(catalog()[placement.componentId].addresses());
        if (counts.size() != assignment.placements.size()) continue;
        const auto offsets = chainOffsets(counts, chainStarts(assignment));
        for (int item = 0; item < offsets.size(); ++item) result.insert(assignment.placements[item], offsets[item]);
    }
    return result;
}

int LightingDocument::outputLedsUsed(const ChannelAssignment& assignment) const
{
    QVector<int> counts;
    int sum = 0;
    for (const auto& id : assignment.placements)
        for (const auto& placement : state_.placements) if (placement.id == id) {
            counts.append(catalog()[placement.componentId].addresses());
            sum += counts.last();
        }
    // Native channels and incomplete lists have no chain positions; fall back to the plain sum.
    if (!assignment.software || counts.size() != assignment.placements.size()) return sum;
    const auto offsets = chainOffsets(counts, chainStarts(assignment));
    int end = 0;
    for (int item = 0; item < offsets.size(); ++item) end = std::max(end, offsets[item] + counts[item]);
    return end;
}

void LightingDocument::setIdentifyPlacement(const QUuid& placement) { identify_ = placement; }

bool LightingDocument::hasSoftwareChannel(const QUuid& placement) const
{
    for (const auto& assignment : state_.assignments) if (assignment.software && assignment.placements.contains(placement)) return true;
    return false;
}

QMap<QUuid, QString> LightingDocument::sharedColorNotes() const
{
    QMap<QUuid, QString> result;
    for (const auto& assignment : state_.assignments) {
        const auto starts = chainStarts(assignment);
        if (!assignment.software || starts.isEmpty()) continue;
        QVector<int> counts; QStringList names;
        for (const auto& id : assignment.placements)
            for (const auto& placement : state_.placements) if (placement.id == id) {
                counts.append(catalog()[placement.componentId].addresses()); names.append(componentName(placement));
            }
        if (counts.size() != assignment.placements.size() || counts.contains(0)) continue;
        const auto sharing = chainSharing(counts, starts);
        for (int item = 0; item < sharing.size(); ++item) {
            const auto note = sharedColorNote(sharing[item], item, names, true);
            if (!note.isEmpty()) result.insert(assignment.placements[item], note);
        }
        for (int item = 0; item < sharing.size(); ++item)
            for (const auto& run : sharing[item])
                if (run.source != item && !result.contains(assignment.placements[run.source]))
                    result.insert(assignment.placements[run.source], "Color source");
    }
    return result;
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
    return cachedRender(seconds,id);
}

QImage LightingDocument::render(double seconds) const { return cachedRender(seconds,{}); }

QImage LightingDocument::cachedRender(double seconds,const QUuid& channel) const
{
    if(!std::isfinite(seconds) || seconds<0 || seconds>86400)throw std::invalid_argument("Invalid preview time");
    if(frameRevision_!=revision_ || frameSeconds_!=seconds){frameCache_.clear();frameRevision_=revision_;frameSeconds_=seconds;}
    const double local=channel.isNull() ? seconds : channelTime(channel,seconds);
    // Only effective time and included root layers affect channel composition. Share
    // immutable rasters across output, Layout and Live Hardware for this exact frame.
    QString key=QString::number(local,'g',17);
    for(const auto& layer:state_.layers)if(layer.parentId.isNull() && (channel.isNull() || layer.allChannels || layer.channelTargets.contains(channel)))key+=':'+layer.id.toString();
    const auto found=frameCache_.constFind(key);if(found!=frameCache_.cend())return *found;
    const auto result=render(state_,local,channel);frameCache_.insert(key,result);return result;
}

void LightingDocument::save(const QString& fileName, bool markClean, const LightingState* state, QJsonArray* playbackDefinitions)
{
    // Autosave must not persist an unaccepted picker preview.
    const auto& saved=state ? *state : colorPreviewBefore_ ? *colorPreviewBefore_ : state_;
    QJsonArray layers, placements, components, assignments;
    validateChannelAssignments(saved, profiles_);
    for (const auto& assignment : saved.assignments) assignments.append(channelAssignmentJson(assignment));
    const bool chase = std::any_of(saved.layers.cbegin(),saved.layers.cend(),[](const auto& layer) { return layer.effect==EffectKind::Chase; });
    const bool references = !libraryPath_.isEmpty() || playbackDefinitions || chase;
    if (!references) {
        for (const auto& component : saved.components) components.append(component.toJson());
    } else {
        ComponentCatalog used;
        for (const auto& placement : saved.placements) {
            const auto definition = saved.components.value(placement.componentId);
            if (!initialCatalog_.contains(definition.id) || initialCatalog_.value(definition.id).toJson() != definition.toJson())
                used.insert(definition.id, definition);
        }
        if (playbackDefinitions) {
            *playbackDefinitions = {};
            for (const auto& definition : used) playbackDefinitions->append(definition.toJson());
        }
        if (!playbackDefinitions && !libraryPath_.isEmpty() && !used.isEmpty()) {
            QLockFile lock(libraryLock(libraryPath_));
            if (!lock.tryLock()) throw std::runtime_error("Custom component library is busy");
            auto custom = readLibrary(libraryPath_, initialCatalog_);
            bool changed = false;
            for (const auto& definition : used) {
                if (custom.contains(definition.id) && custom.value(definition.id).toJson() != definition.toJson())
                    throw std::invalid_argument("Save geometry as a new component variant before overwriting a definition");
                if (!custom.contains(definition.id)) {
                    custom.insert(definition.id, definition);
                    changed = true;
                }
            }
            if (changed) writeLibrary(libraryPath_, custom);
        }
    }
    for (const auto& layer : saved.layers) {
        QByteArray png;
        QBuffer buffer(&png); buffer.open(QIODevice::WriteOnly);
        if (!layer.mask.save(&buffer, "PNG")) throw std::runtime_error("Could not encode layer mask");
        QByteArray imagePng;
        if (!layer.image.isNull()) {
            QBuffer imageBuffer(&imagePng); imageBuffer.open(QIODevice::WriteOnly);
            if (!layer.image.save(&imageBuffer, "PNG")) throw std::runtime_error("Could not encode image");
        }
        QJsonObject advanced;
        for (const auto& parameter : advancedParameters()) if (chase || parameter.schemaVersion<26) advanced.insert(parameter.id, layer.*(parameter.member));
        QJsonArray cycle;
        for (const auto& color : layer.colorCycle) cycle.append(color.name(QColor::HexArgb));
        QJsonArray cyclePositions; for(double position:layer.colorCyclePositions) cyclePositions.append(position);
        if(!cyclePositions.isEmpty()) advanced.insert("colorCyclePositions",cyclePositions);
        advanced.insert("colorCycle", cycle); advanced.insert("maskLinked", layer.maskLinked); if(layer.clipToFrame) advanced.insert("clipToFrame", true);
        advanced.insert("crop", QJsonArray{layer.crop.x(), layer.crop.y(), layer.crop.width(), layer.crop.height()});
        QJsonArray positions; for(double position:layer.gradientPositions) positions.append(position);
        QJsonArray gradient; for(const auto& color:layer.gradientColors) gradient.append(color.name(QColor::HexArgb));
        QJsonArray targets; for (const auto& id : layer.channelTargets) targets.append(id.toString());
        layers.append(QJsonObject{{"id", layer.id.toString()}, {"parentId",layer.parentId.isNull() ? QJsonValue(QJsonValue::Null) : QJsonValue(layer.parentId.toString())}, {"container",layer.container},
            {"gradientPositions",positions}, {"gradientColors", gradient}, {"targets", layer.allChannels ? QJsonValue(QJsonValue::Null) : QJsonValue(targets)}, {"name", layer.name},
            {"color", layer.color.name(QColor::HexArgb)}, {"visible", layer.visible}, {"locked",layer.locked},
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
    for (const auto& placement : saved.placements)
        placements.append(QJsonObject{{"id", placement.id.toString()}, {"componentId", placement.componentId}, {"locked",placement.locked},
            {"x", placement.origin.x()}, {"y", placement.origin.y()},
            {"scaleX", placement.scale.width()}, {"scaleY", placement.scale.height()}, {"rotation", placement.rotation}});
    // Version 14 persists editing locks; older readers must reject rather than silently unlock content.
    // Version 15 adds cycle stop timing; older readers must not silently discard those positions.
    // Version 16 adds native patterns and their controls; earlier parameter defaults remain readable.
    // Version 20 adds branching Lightning and perspective Starfield sources.
    // Version 22 adds procedural spatial controls; old files retain their original defaults.
    // Version 23 adds shared chain positions. Files without them stay at 22 so an
    // installed service host that predates this change keeps loading existing projects.
    const bool sharedPositions = std::any_of(saved.assignments.cbegin(), saved.assignments.cend(),
        [](const auto& assignment) { return !chainStarts(assignment).isEmpty(); });
    // Version 24 adds opt-in frame clipping (PR #9 review); older hosts must reject rather than render unclipped.
    const bool clipped = std::any_of(saved.layers.cbegin(), saved.layers.cend(), [](const auto& layer) { return layer.clipToFrame; });
    // Schema 25 resolves component IDs from built-ins and the user's separate library.
    QJsonObject project{{"schemaVersion", chase ? 26 : references ? 25 : clipped ? 24 : sharedPositions ? 23 : 22}, {"width", 640}, {"height", 400},
        {"layers", layers}, {"placements", placements}, {"assignments", assignments}};
    if (!references) project.insert("components", components);
    const auto bytes = QJsonDocument(project).toJson();
    if (bytes.size() > 32 * 1024 * 1024) throw std::invalid_argument("Document exceeds 32 MiB; reduce images before saving");
    QSaveFile file(fileName);
    // Atomic replacement preserves an existing project if writing/commit fails.
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit())
        throw std::runtime_error("Could not save lighting document");
    // A supplied state is not the document's own; the file then does not match the history (CodeRabbit review 2026-11-18).
    if (markClean && !state) history_.setClean();
}

void LightingDocument::load(const QString& fileName, const QJsonArray& playbackDefinitions)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly) || file.size() > 32 * 1024 * 1024)
        throw std::invalid_argument("Document unreadable or larger than 32 MiB");
    QJsonParseError error;
    const auto parsed = QJsonDocument::fromJson(file.read(32 * 1024 * 1024 + 1), &error);
    const auto root = parsed.object();
    const double schema = root.value("schemaVersion").toDouble(-1);
    if (error.error != QJsonParseError::NoError || !parsed.isObject() ||
        (schema != 1 && schema != 2 && schema != 3 && schema != 4 && schema != 5 && schema != 6 && schema != 7 && schema != 8 && schema != 9 && schema != 10 && schema != 11 && schema != 12 && schema != 13 && schema != 14 && schema != 15 && schema != 16 && schema != 17 && schema != 18 && schema != 19 && schema != 20 && schema != 21 && schema != 22 && schema != 23 && schema != 24 && schema != 25 && schema != 26) || root.value("width").toDouble(-1) != 640 ||
        root.value("height").toDouble(-1) != 400 || !root.value("layers").isArray() || !root.value("placements").isArray())
        throw std::invalid_argument("Invalid lighting document header");
    const auto layerArray = root.value("layers").toArray();
    const auto placementArray = root.value("placements").toArray();
    if (layerArray.size() > 64 || placementArray.size() > 256)
        throw std::invalid_argument("Document exceeds layer/component limits");
    LightingState candidate;
    candidate.components = initialCatalog_;
    QMap<QString, QString> migratedIds;
    std::unique_ptr<QLockFile> migrationLock;
    auto custom = readLibrary(libraryPath_, initialCatalog_);
    bool migrated = false;
    if (schema >= 25) {
        if (root.contains("components")) throw std::invalid_argument("Referenced projects cannot embed component definitions");
        if (playbackDefinitions.size() > 256) throw std::invalid_argument("Too many playback component definitions");
        QSet<QString> bundled;
        for (const auto& entry : playbackDefinitions) {
            const auto definition = ComponentDefinition::fromJson(entry.toObject());
            if (bundled.contains(definition.id) || initialCatalog_.contains(definition.id))
                throw std::invalid_argument("Conflicting playback component ID");
            bundled.insert(definition.id);
            if (!libraryPath_.isEmpty() && custom.contains(definition.id) && custom.value(definition.id).toJson() != definition.toJson()) {
                // The Component Editor saves library entries in place, so the service can still play an
                // older definition under the same ID. Keep what is playing for this project under a
                // content-derived variant (local copy only; the library keeps the user's edit).
                migratedIds.insert(definition.id, reconcileDefinition(definition, initialCatalog_, custom));
                continue;
            }
            custom.insert(definition.id, definition);
        }
        for (const auto& definition : custom) if (!initialCatalog_.contains(definition.id))
            candidate.components.insert(definition.id, definition);
    } else if (schema >= 7) {
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
    if (schema >= 7 && schema < 25 && !libraryPath_.isEmpty()) {
        migrationLock = std::make_unique<QLockFile>(libraryLock(libraryPath_));
        if (!migrationLock->tryLock()) throw std::runtime_error("Custom component library is busy");
        custom = readLibrary(libraryPath_, initialCatalog_);
        for (const auto& definition : candidate.components)
            migratedIds.insert(definition.id, reconcileDefinition(definition, initialCatalog_, custom));
        candidate.components = initialCatalog_;
        for (const auto& definition : custom) candidate.components.insert(definition.id, definition);
        migrated = true;
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
        if(schema>=14){if(!json.value("locked").isBool())throw std::invalid_argument("Invalid object lock");layer.locked=json.value("locked").toBool();}
        else if(json.contains("locked"))throw std::invalid_argument("Object locks require schema 14");
        layer.color = QColor(json.value("color").toString()); layer.visible = json.value("visible").toBool();
        layer.transparency = static_cast<int>(transparency); layer.blend = static_cast<LayerBlend>(static_cast<int>(blend));
        layer.maskEnabled = json.value("maskEnabled").toBool();
        if (schema >= 2) {
            const auto effect = json.value("effect").toObject();
            for (const auto& key : {"centerX", "centerY", "width", "height", "rotation", "repetitions"})
                if (!effect.value(key).isDouble()) throw std::invalid_argument("Missing numeric effect parameter");
            layer.effect = effectDefinition(effect.value("type").toString()).kind;
            // Advanced values arrive later; early geometry validation needs kind-specific defaults.
            layer.particleCount=effectDefinition(layer.effect).particleCount;
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
        if(schema<26 && layer.effect==EffectKind::Chase) throw std::invalid_argument("Chase requires schema 26");
        if(schema<20 && (layer.effect==EffectKind::Lightning || layer.effect==EffectKind::Starfield)) throw std::invalid_argument("Lightning and Starfield require schema 20");
        if(schema<19 && layer.effect>=EffectKind::Aurora && layer.effect<=EffectKind::Kaleidoscope) throw std::invalid_argument("Additional procedural sources require schema 19");
        if(schema<18 && (layer.effect==EffectKind::Plasma || layer.effect==EffectKind::Ripples)) throw std::invalid_argument("Plasma and Ripples require schema 18");
        if(schema<17 && (layer.effect>=EffectKind::Voronoi && layer.effect<=EffectKind::Fireworks)) throw std::invalid_argument("Extended effects require schema 17");
        if(schema<16 && isPattern(layer.effect)) throw std::invalid_argument("Native patterns require schema 16");
        if(schema<13 && layer.effect==EffectKind::Rectangle)throw std::invalid_argument("Editable rectangles require schema 13");
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
                if(schema<parameter.schemaVersion) continue;
                if (!advanced.value(parameter.id).isDouble()) throw std::invalid_argument("Missing advanced parameter");
                layer.*(parameter.member) = advanced.value(parameter.id).toDouble();
            }
            if (!advanced.value("maskLinked").isBool() || !advanced.value("colorCycle").isArray())
                throw std::invalid_argument("Invalid linked mask/color cycle");
            layer.maskLinked = advanced.value("maskLinked").toBool(); layer.colorCycle.clear();
            if(advanced.contains("clipToFrame") && (schema<24 || !advanced.value("clipToFrame").isBool())) throw std::invalid_argument("Frame clipping requires schema 24");
            layer.clipToFrame = advanced.value("clipToFrame").toBool(false);
            const auto cycle = advanced.value("colorCycle").toArray();
            if (cycle.size() < 2 || cycle.size() > 16) throw std::invalid_argument("Invalid color cycle size");
            for (const auto& color : cycle) layer.colorCycle.append(QColor(color.toString()));
            if(advanced.contains("colorCyclePositions")) {
                if(schema<15 || !advanced.value("colorCyclePositions").isArray()) throw std::invalid_argument("Cycle positions require schema 15");
                for(const auto& value:advanced.value("colorCyclePositions").toArray()) {
                    if(!value.isDouble()) throw std::invalid_argument("Invalid cycle position");
                    layer.colorCyclePositions.append(value.toDouble());
                }
            }
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
        if(schema>=13) {
            if(!json.contains("parentId") || (!json.value("parentId").isNull() && !json.value("parentId").isString()) || !json.value("container").isBool())throw std::invalid_argument("Invalid layer/object metadata");
            if(!json.value("parentId").isNull()) {layer.parentId=QUuid(json.value("parentId").toString());if(layer.parentId.isNull())throw std::invalid_argument("Invalid parent layer UUID");}
            layer.container=json.value("container").toBool();
        }else if(json.contains("parentId") || json.contains("container"))throw std::invalid_argument("Layer objects require schema 13");
        candidate.layers.append(layer);
    }
    QStringList missing;
    for (const auto& entry : placementArray) {
        const auto json = entry.toObject();
        if (!json.value("x").isDouble() || !json.value("y").isDouble())
            throw std::invalid_argument("Invalid placement coordinates");
        ComponentPlacement placement;
        placement.id = QUuid(json.value("id").toString());
        const auto sourceId = json.value("componentId").toString();
        placement.componentId = migratedIds.value(sourceId, sourceId);
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
        if(schema>=14){if(!json.value("locked").isBool())throw std::invalid_argument("Invalid component lock");placement.locked=json.value("locked").toBool();}
        else if(json.contains("locked"))throw std::invalid_argument("Component locks require schema 14");
        if (!candidate.components.contains(placement.componentId)) missing.append(placement.componentId);
        candidate.placements.append(placement);
    }
    missing.removeDuplicates();
    if (!missing.isEmpty()) throw MissingComponents(missing);
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
    if (migrated) writeLibrary(libraryPath_, custom);
    // Parse/validate everything before replacing the live document and undo history.
    state_ = std::move(candidate);
    history_.clear();
    history_.setClean();
    ++revision_; emit changed();
}

void LightingDocument::saveCustomEffect(const QUuid& selection, const QString& fileName)
{
    LightingState preset;
    for (auto layer : state_.layers) if (layer.id==selection || layer.parentId==selection) {
        if (layer.id==selection) layer.parentId={};
        layer.allChannels=true;
        layer.channelTargets.clear();
        preset.layers.append(layer);
    }
    if (selection.isNull() || preset.layers.isEmpty()) throw std::invalid_argument("Select a layer or group first");
    save(fileName,false,&preset);
}

QUuid LightingDocument::insertCustomEffect(const QString& fileName)
{
    LightingDocument preset(initialCatalog_);
    preset.load(fileName);
    auto layers = preset.state().layers;
    QUuid root;
    QMap<QUuid,QUuid> ids;
    for (const auto& layer : layers) {
        if (layer.parentId.isNull()) {
            if (!root.isNull()) throw std::invalid_argument("Custom effect must have one root layer");
            root=layer.id;
        }
        ids.insert(layer.id,QUuid::createUuid());
    }
    if (root.isNull() || !preset.state().placements.isEmpty()) throw std::invalid_argument("Invalid custom effect");
    edit("Insert custom effect",[&](LightingState& state) {
        for (auto& layer : layers) {
            layer.id=ids.value(layer.id);
            layer.parentId=ids.value(layer.parentId);
            layer.locked=false;
            layer.allChannels=true;
            layer.channelTargets.clear();
            state.layers.append(layer);
        }
    });
    return ids.value(root);
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
    return composeLighting(state.layers,seconds,channel);
}

namespace {
QImage composeLighting(const QVector<EffectLayer>& layers, double seconds, const QUuid& channel,
                       const QVector<QPoint>* addresses)
{
    // An address atlas preserves independent addresses even when their points overlap on the canvas.
    const auto size = addresses ? QSize(addresses->size(),1) : LightingDocument::canvasSize();
    QImage result(size,QImage::Format_ARGB32_Premultiplied);
    result.fill(Qt::transparent);
    QPainter painter(&result);
    for (const auto& layer : layers) {
        if (!layer.parentId.isNull() || !layer.visible || layer.transparency == 100 ||
            (!channel.isNull() && !layer.allChannels && !layer.channelTargets.contains(channel))) continue;
        QImage source;
        const bool children=std::any_of(layers.cbegin(),layers.cend(),[&](const auto& item){return item.parentId==layer.id;});
        if(children) {
            QVector<EffectLayer> group;
            if(!layer.container){auto own=layer;own.parentId={};own.maskEnabled=false;own.transparency=0;own.blend=LayerBlend::Normal;group.append(own);}
            for(auto child:layers)if(child.parentId==layer.id){child.parentId={};child.allChannels=true;group.append(child);}
            source=composeLighting(group,seconds,{},addresses);
        } else if (layer.container || (layer.effect==EffectKind::Chase && addresses)) {
            source=QImage(size,QImage::Format_ARGB32_Premultiplied);
            source.fill(Qt::transparent);
            if (!layer.container) {
                const int count=size.width();
                const int step=int(std::floor(seconds*layer.chaseSpeed))%count;
                const int head=layer.chaseReverse ? count-1-step : step;
                auto* pixels=reinterpret_cast<QRgb*>(source.scanLine(0));
                for (int index=0;index<qMin(count,int(layer.chaseLength));++index) {
                    const int address=(head+(layer.chaseReverse ? index : -index)+count)%count;
                    if (addresses->at(address).x()>=0) pixels[address]=qPremultiply(layer.color.rgba());
                }
            }
        } else {
            const auto raster=renderEffect(layer,seconds);
            if (!addresses) source=raster;
            else {
                source=QImage(size,QImage::Format_ARGB32_Premultiplied);
                auto* pixels=reinterpret_cast<QRgb*>(source.scanLine(0));
                for (int address=0;address<size.width();++address) {
                    const auto point=addresses->at(address);
                    pixels[address]=point.x()>=0 ? raster.pixel(point) : 0;
                }
            }
        }
        const auto maskImage = renderMask(layer,seconds);
        for (int y = 0; y < source.height(); ++y) {
            auto* pixels = reinterpret_cast<QRgb*>(source.scanLine(y));
            const auto* mask = maskImage.constScanLine(y);
            for (int x = 0; x < source.width(); ++x) {
                const auto point=addresses ? addresses->at(x) : QPoint(x,y);
                const int coverage = !layer.maskEnabled ? 255 :
                    point.x()<0 ? 0 : addresses ? maskImage.constScanLine(point.y())[point.x()] : mask[x];
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
}
