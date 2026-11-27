#include "ComponentCatalog.h"

#include <QDir>
#include <QHash>
#include <algorithm>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSet>
#include <QLineF>
#include <cmath>
#include <stdexcept>

namespace {
int integer(const QJsonValue& value, int minimum, int maximum)
{
    const double number = value.toDouble(-1);
    if (!value.isDouble() || !std::isfinite(number) || std::floor(number) != number ||
        number < minimum || number > maximum)
        throw std::invalid_argument("Invalid component integer");
    return static_cast<int>(number);
}
QString requiredText(const QJsonObject& json, const QString& key)
{
    const auto value = json.value(key);
    if (!value.isString() || value.toString().trimmed().isEmpty())
        throw std::invalid_argument(("Missing component string: " + key).toStdString());
    return value.toString().trimmed();
}
}

ComponentDefinition ComponentDefinition::fromJson(const QJsonObject& json)
{
    const int schema = integer(json.value("schemaVersion"), 1, 2);
    ComponentDefinition component;
    component.id = requiredText(json, "id");
    component.name = requiredText(json, "name");
    for(const auto* key:{"manufacturer","category"}) if(json.contains(key) &&
        (!json.value(key).isString() || json.value(key).toString().trimmed().isEmpty() || json.value(key).toString().size()>160))
        throw std::invalid_argument("Invalid component identity");
    component.manufacturer=json.value("manufacturer").toString().trimmed();
    component.category=json.value("category").toString().trimmed();
    if(json.contains("verification")) {
        component.verification=json.value("verification").toString();
        if(!QStringList{"verified","manufacturer","unverified"}.contains(component.verification))
            throw std::invalid_argument("Invalid component verification");
    }
    const int count = integer(json.value("ledCount"), 1, 65535);
    if (!json.value("mapping").isArray() || !json.value("positions").isArray())
        throw std::invalid_argument("Missing component LED arrays");
    const auto mapping = json.value("mapping").toArray();
    const auto positions = json.value("positions").toArray();
    if (mapping.size() != count || positions.size() != count)
        throw std::invalid_argument("Component LED array length mismatch");
    if (schema == 2) component.addressCount = integer(json.value("addressCount"), 1, count);
    else if (json.contains("addressCount")) throw std::invalid_argument("Shared LED addresses require component schema 2");
    const int addresses = component.addressCount ? component.addressCount : count;
    QSet<int> seen;
    for (int i = 0; i < count; ++i) {
        const int wire = integer(mapping[i], 0, addresses - 1);
        if (schema == 1 && seen.contains(wire)) throw std::invalid_argument("Duplicate component wire index");
        seen.insert(wire);
        component.mapping.append(wire);
        if (!positions[i].isArray()) throw std::invalid_argument("Invalid LED position");
        const auto point = positions[i].toArray();
        if (point.size() != 2 || !point[0].isDouble() || !point[1].isDouble() ||
            !std::isfinite(point[0].toDouble()) || !std::isfinite(point[1].toDouble()))
            throw std::invalid_argument("LED position requires two finite coordinates");
        component.positions.append(QPointF(point[0].toDouble(), point[1].toDouble()));
    }
    if (seen.size() != addresses) throw std::invalid_argument("Every address needs a position");
    if (json.contains("nonPhysicalSlots")) {
        if (!json.value("nonPhysicalSlots").isArray()) throw std::invalid_argument("Invalid non-physical slots");
        QSet<int> reserved;
        for (const auto& value : json.value("nonPhysicalSlots").toArray()) {
            const int wire = integer(value, 0, addresses - 1);
            if (reserved.contains(wire)) throw std::invalid_argument("Duplicate non-physical slot");
            reserved.insert(wire); component.nonPhysicalSlots.append(wire);
        }
        if (component.nonPhysicalSlots.size() == addresses) throw std::invalid_argument("Component needs a physical LED");
    } else if (count == 144 && (component.id == "peripheral.vulcan" ||
               (component.id.startsWith("instance.") && component.name == "Roccat Vulcan 100/120 Aimo"))) {
        // Older embedded Vulcan definitions predate explicit reserved slots; Esc wire 0 is physical.
        component.nonPhysicalSlots = {22, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 58, 64, 71, 77, 81, 90, 91, 92, 93, 94, 95, 97, 98, 112, 118, 123, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143};
    }
    if (json.contains("layers")) {
        if (!json.value("layers").isArray() || json.value("layers").toArray().size() != count)
            throw std::invalid_argument("Invalid LED layers");
        bool any = false;
        for (const auto& value : json.value("layers").toArray()) {
            component.layers.append(integer(value, 0, 15)); any = any || component.layers.back() > 0;
        }
        if (!any) component.layers.clear();
    }
    if (json.contains("housing")) {
        const auto box = json.value("housing").toArray();
        QVector<double> v; for (const auto& value : box) v.append(value.toDouble(qQNaN()));
        if (!json.value("housing").isArray() || v.size() != 4 || std::any_of(v.cbegin(), v.cend(), [](double x) { return !std::isfinite(x) || std::abs(x) > 1e6; })
            || v[2] <= 0 || v[3] <= 0)
            throw std::invalid_argument("Invalid component housing");
        component.housing = QRectF(v[0], v[1], v[2], v[3]);
    }
    if(count==15 && component.name.contains("D30-120",Qt::CaseInsensitive)) {
        const QVector<int> oldX{2,1,1,0,0,0,0,0,0,0,0,0,1,1,2};
        bool legacy=true;
        for(int i=0;i<count;++i) if(QLineF(component.positions[i],QPointF(oldX[i]*3.0/14,i*3.0/14)).length()>1e-7) legacy=false;
        // Migrate only the exact imported strip-shaped D30 template; retain edited layouts and wire order.
        if(legacy) for(int i=0;i<count;++i) {
            const double angle=6.283185307179586*i/count;
            component.positions[i]={1.3+1.3*std::cos(angle),1.3+1.3*std::sin(angle)};
        }
    }
    return component;
}

QJsonObject ComponentDefinition::toJson() const
{
    QJsonArray points, wires;
    for (const auto& point : positions) points.append(QJsonArray{point.x(), point.y()});
    for (const auto wire : mapping) wires.append(wire);
    QJsonObject result{{"schemaVersion", addressCount ? 2 : 1}, {"id", id}, {"name", name},
        {"ledCount", positions.size()}, {"positions", points}, {"mapping", wires}};
    if (addressCount) result.insert("addressCount", addressCount);
    if(!manufacturer.isEmpty()) result.insert("manufacturer",manufacturer);
    if(!category.isEmpty()) result.insert("category",category);
    if(!verification.isEmpty()) result.insert("verification",verification);
    if (std::any_of(layers.cbegin(), layers.cend(), [](int layer) { return layer > 0; })) {
        QJsonArray values; for (int layer : layers) values.append(layer);
        result.insert("layers", values);
    }
    if (!nonPhysicalSlots.isEmpty()) {
        QJsonArray reserved; for (int wire : nonPhysicalSlots) reserved.append(wire);
        result.insert("nonPhysicalSlots", reserved);
    }
    if (housing.isValid()) result.insert("housing", QJsonArray{housing.x(), housing.y(), housing.width(), housing.height()});
    return result;
}

QVector<int> overlapLayers(const QVector<QPointF>& positions)
{
    QVector<int> layers; QHash<QPair<double, double>, int> seen; bool any = false;
    for (const auto& point : positions) {
        const int layer = seen[{point.x(), point.y()}]++;
        layers.append(layer); any = any || layer > 0;
    }
    return any ? layers : QVector<int>{};
}

ComponentDefinition loadComponentDefinition(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) throw std::invalid_argument("Cannot read component");
    // Bound reads even if the file grows after opening.
    const auto bytes = file.read(1024 * 1024 + 1);
    if (bytes.size() > 1024 * 1024 || file.error() != QFileDevice::NoError)
        throw std::invalid_argument("Component read failed or exceeds 1 MiB");
    QJsonParseError error;
    const auto json = QJsonDocument::fromJson(bytes, &error);
    if (error.error != QJsonParseError::NoError || !json.isObject())
        throw std::invalid_argument("Malformed component JSON");
    return ComponentDefinition::fromJson(json.object());
}

ComponentCatalog loadComponentCatalog(const QString& directory)
{
    const QDir dir(directory);
    if (!dir.exists()) throw std::invalid_argument("Component directory does not exist");
    ComponentCatalog catalog;
    for (const auto& entry : dir.entryInfoList({"*.json"}, QDir::Files, QDir::Name)) {
        try {
            const auto component = loadComponentDefinition(entry.absoluteFilePath());
            if (catalog.contains(component.id)) throw std::invalid_argument("Duplicate component ID");
            catalog.insert(component.id, component);
        } catch (const std::exception& error) {
            throw std::invalid_argument((entry.fileName() + ": " + error.what()).toStdString());
        }
    }
    return catalog;
}

QVector<int> chainOffsets(const QVector<int>& counts, const QVector<int>& starts)
{
    if (!starts.isEmpty() && starts.size() != counts.size())
        throw std::invalid_argument("Chain positions do not match the component chain");
    QVector<int> offsets;
    int cursor = 0;
    for (qsizetype i = 0; i < counts.size(); ++i) {
        const int start = starts.value(i, 0);
        if (start < 0 || start > 65535 || counts[i] < 1 || counts[i] > 65535)
            throw std::invalid_argument("Invalid chain position");
        offsets.append(start ? start - 1 : cursor);
        cursor = offsets.last() + counts[i];
    }
    return offsets;
}

QVector<QVector<SharedRun>> chainSharing(const QVector<int>& counts, const QVector<int>& starts)
{
    const auto offsets = chainOffsets(counts, starts);
    QVector<int> owner;
    for (qsizetype item = 0; item < counts.size(); ++item) {
        if (owner.size() < offsets[item] + counts[item]) owner.resize(offsets[item] + counts[item], -1);
        for (int led = 0; led < counts[item]; ++led)
            if (owner[offsets[item] + led] < 0) owner[offsets[item] + led] = int(item);
    }
    QVector<QVector<SharedRun>> result(counts.size());
    for (qsizetype item = 0; item < counts.size(); ++item)
        for (int led = 0; led < counts[item]; ++led) {
            const int source = owner[offsets[item] + led];
            auto& runs = result[item];
            if (!runs.isEmpty() && runs.last().source == source) runs.last().lastLed = led + 1;
            else runs.append({led + 1, led + 1, source});
        }
    return result;
}

QVector<int> channelMapping(const ChannelProfile& channel, const QStringList& componentIds,
                           const ComponentCatalog& catalog, const QVector<int>& starts)
{
    if (channel.maxLeds < 1 || channel.maxLeds > 65535 || channel.fixedLeds < 0 ||
        channel.fixedLeds > channel.maxLeds)
        throw std::invalid_argument("Invalid channel capacity");
    QVector<int> counts;
    for (const auto& id : componentIds) {
        const auto component = catalog.constFind(id);
        if (component == catalog.cend()) throw std::invalid_argument("Unknown component ID");
        counts.append(component->addresses());
        if (counts.last() < 1) throw std::invalid_argument("Component chain exceeds channel capacity");
    }
    const auto offsets = chainOffsets(counts, starts);
    QVector<int> result;
    int extent = 0;
    for (qsizetype index = 0; index < componentIds.size(); ++index) {
        const auto component = catalog.constFind(componentIds[index]);
        const int count = counts[index], offset = offsets[index];
        if (count > channel.maxLeds - offset)
            throw std::invalid_argument("Component chain exceeds channel capacity");
        extent = qMax(extent, offset + count);
        QSet<int> seen;
        // Definitions can also be assembled in memory; validate before indexing frames.
        for (const int wire : component->mapping) {
            if (wire < 0 || wire >= count || (!component->addressCount && seen.contains(wire)))
                throw std::invalid_argument("Invalid component wire mapping");
            seen.insert(wire);
            result.append(offset + wire);
        }
        if (seen.size() != count) throw std::invalid_argument("Missing component wire address");
    }
    // Shared positions count once: the fixed size is the occupied address range.
    if (channel.fixedLeds && extent != channel.fixedLeds)
        throw std::invalid_argument("Component chain differs from fixed channel count");
    return result;
}
