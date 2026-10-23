#include "ComponentCatalog.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSet>
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
    integer(json.value("schemaVersion"), 1, 1);
    ComponentDefinition component;
    component.id = requiredText(json, "id");
    component.name = requiredText(json, "name");
    const int count = integer(json.value("ledCount"), 1, 65535);
    if (!json.value("mapping").isArray() || !json.value("positions").isArray())
        throw std::invalid_argument("Missing component LED arrays");
    const auto mapping = json.value("mapping").toArray();
    const auto positions = json.value("positions").toArray();
    if (mapping.size() != count || positions.size() != count)
        throw std::invalid_argument("Component LED array length mismatch");
    QSet<int> seen;
    for (int i = 0; i < count; ++i) {
        const int wire = integer(mapping[i], 0, count - 1);
        if (seen.contains(wire)) throw std::invalid_argument("Duplicate component wire index");
        seen.insert(wire);
        component.mapping.append(wire);
        if (!positions[i].isArray()) throw std::invalid_argument("Invalid LED position");
        const auto point = positions[i].toArray();
        if (point.size() != 2 || !point[0].isDouble() || !point[1].isDouble() ||
            !std::isfinite(point[0].toDouble()) || !std::isfinite(point[1].toDouble()))
            throw std::invalid_argument("LED position requires two finite coordinates");
        component.positions.append(QPointF(point[0].toDouble(), point[1].toDouble()));
    }
    return component;
}

QJsonObject ComponentDefinition::toJson() const
{
    QJsonArray points, wires;
    for (const auto& point : positions) points.append(QJsonArray{point.x(), point.y()});
    for (const auto wire : mapping) wires.append(wire);
    return {{"schemaVersion", 1}, {"id", id}, {"name", name},
        {"ledCount", positions.size()}, {"positions", points}, {"mapping", wires}};
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

QVector<int> channelMapping(const ChannelProfile& channel, const QStringList& componentIds,
                           const ComponentCatalog& catalog)
{
    if (channel.maxLeds < 1 || channel.maxLeds > 65535 || channel.fixedLeds < 0 ||
        channel.fixedLeds > channel.maxLeds)
        throw std::invalid_argument("Invalid channel capacity");
    QVector<int> result;
    for (const auto& id : componentIds) {
        const auto component = catalog.constFind(id);
        if (component == catalog.cend()) throw std::invalid_argument("Unknown component ID");
        const int count = static_cast<int>(component->mapping.size());
        if (count < 1 || count > channel.maxLeds - result.size())
            throw std::invalid_argument("Component chain exceeds channel capacity");
        const int offset = static_cast<int>(result.size());
        QSet<int> seen;
        // Definitions can also be assembled in memory; validate before indexing frames.
        for (const int wire : component->mapping) {
            if (wire < 0 || wire >= count || seen.contains(wire))
                throw std::invalid_argument("Invalid component wire mapping");
            seen.insert(wire);
            result.append(offset + wire);
        }
    }
    if (channel.fixedLeds && result.size() != channel.fixedLeds)
        throw std::invalid_argument("Component chain differs from fixed channel count");
    return result;
}
