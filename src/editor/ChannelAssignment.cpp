#include "ChannelAssignment.h"
#include "LightingDocument.h"
#include <QJsonArray>
#include <QSet>
#include <cmath>
#include <stdexcept>

const DeviceProfile& assignmentProfile(const QVector<DeviceProfile>& profiles, const QString& id)
{
    for (const auto& profile : profiles) if (profile.id == id) return profile;
    throw std::invalid_argument("Assignment requires an unavailable device profile");
}
const ChannelProfile& assignmentChannel(const DeviceProfile& profile, const QString& id)
{
    for (const auto& channel : profile.channels) if (channel.id == id) return channel;
    throw std::invalid_argument("Unknown channel for device profile");
}
void validateChannelAssignments(const LightingState& state, const QVector<DeviceProfile>& profiles)
{
    if (state.assignments.size() > 256) throw std::invalid_argument("Limit: 256 channel assignments");
    QSet<QUuid> ids, usedPlacements;
    for (const auto& placement : state.placements) ids.insert(placement.id);
    for (const auto& layer : state.layers) ids.insert(layer.id);
    QMap<QString, QString> endpointProfiles;
    QMap<QString, QSet<QString>> endpointChannels;
    for (const auto& assignment : state.assignments) {
        if (assignment.id.isNull() || ids.contains(assignment.id) || assignment.endpoint.trimmed().isEmpty())
            throw std::invalid_argument("Invalid assignment identity or endpoint");
        ids.insert(assignment.id);
        if (endpointProfiles.contains(assignment.endpoint) && endpointProfiles[assignment.endpoint] != assignment.profileId)
            throw std::invalid_argument("An endpoint can use only one profile");
        endpointProfiles.insert(assignment.endpoint, assignment.profileId);
        if (endpointChannels[assignment.endpoint].contains(assignment.channelId))
            throw std::invalid_argument("Duplicate endpoint/channel assignment");
        endpointChannels[assignment.endpoint].insert(assignment.channelId);
        const auto& channel = assignmentChannel(assignmentProfile(profiles, assignment.profileId), assignment.channelId);
        if (assignment.software && (!channel.direct || assignment.placements.isEmpty()))
            throw std::invalid_argument("Software output requires a direct-capable channel and a component chain");
        if (!assignment.software && !channel.nativeModes.contains(assignment.native.id))
            throw std::invalid_argument("Unsupported native mode for this channel");
        if (assignment.native.colors.isEmpty() || assignment.native.colors.size() > 4 ||
            assignment.native.brightness < 0 || assignment.native.brightness > 5 ||
            assignment.native.speed < 0 || assignment.native.speed > 2)
            throw std::invalid_argument("Invalid native colors, brightness or speed");
        QStringList components;
        for (const auto& id : assignment.placements) {
            if (usedPlacements.contains(id)) throw std::invalid_argument("A component instance can belong to only one channel");
            const ComponentPlacement* found = nullptr;
            for (const auto& placement : state.placements) if (placement.id == id) found = &placement;
            if (!found) throw std::invalid_argument("Channel refers to an unavailable component instance");
            usedPlacements.insert(id); components.append(found->componentId);
            const auto definition = state.components.constFind(found->componentId);
            if (definition == state.components.cend() || definition->mapping.size() != definition->positions.size())
                throw std::invalid_argument("Invalid assigned component geometry");
        }
        // Native channels may have no geometry; an explicit chain must satisfy the physical capacity.
        if (!components.isEmpty() || assignment.software) channelMapping(channel, components, state.components);
    }
}
QJsonObject channelAssignmentJson(const ChannelAssignment& assignment)
{
    QJsonArray placements, colors;
    for (const auto& id : assignment.placements) placements.append(id.toString());
    for (const auto& color : assignment.native.colors) colors.append(QJsonArray{color.r, color.g, color.b});
    return {{"id", assignment.id.toString()}, {"endpoint", assignment.endpoint},
        {"profileId", assignment.profileId}, {"channelId", assignment.channelId},
        {"software", assignment.software}, {"placements", placements},
        {"native", QJsonObject{{"id", assignment.native.id}, {"colors", colors},
            {"brightness", assignment.native.brightness}, {"speed", assignment.native.speed}, {"inward", assignment.native.inward}}}};
}
ChannelAssignment parseChannelAssignment(const QJsonObject& json)
{
    for (const auto& key : {"id", "endpoint", "profileId", "channelId"})
        if (!json.value(key).isString()) throw std::invalid_argument("Invalid assignment string");
    if (!json.value("software").isBool() || !json.value("placements").isArray() || !json.value("native").isObject())
        throw std::invalid_argument("Invalid assignment fields");
    ChannelAssignment result;
    result.id = QUuid(json.value("id").toString()); result.endpoint = json.value("endpoint").toString();
    result.profileId = json.value("profileId").toString(); result.channelId = json.value("channelId").toString();
    result.software = json.value("software").toBool();
    const auto placements = json.value("placements").toArray();
    if (placements.size() > 256) throw std::invalid_argument("Assignment chain too long");
    for (const auto& entry : placements) {
        if (!entry.isString()) throw std::invalid_argument("Invalid assigned placement ID");
        result.placements.append(QUuid(entry.toString()));
    }
    const auto native = json.value("native").toObject();
    if (!native.value("id").isString() || !native.value("inward").isBool() || !native.value("colors").isArray())
        throw std::invalid_argument("Invalid native effect fields");
    auto integer = [](const QJsonValue& value, int maximum) {
        const double number = value.toDouble(-1);
        if (!value.isDouble() || !std::isfinite(number) || number < 0 || number > maximum || std::floor(number) != number)
            throw std::invalid_argument("Invalid native effect integer");
        return int(number);
    };
    result.native.id = native.value("id").toString(); result.native.inward = native.value("inward").toBool();
    result.native.brightness = integer(native.value("brightness"), 5);
    result.native.speed = integer(native.value("speed"), 2);
    result.native.colors.clear();
    const auto colors = native.value("colors").toArray();
    if (colors.isEmpty() || colors.size() > 4) throw std::invalid_argument("Native effect requires 1..4 RGB colors");
    for (const auto& entry : colors) {
        if (!entry.isArray() || entry.toArray().size() != 3) throw std::invalid_argument("Invalid native RGB color");
        const auto rgb = entry.toArray();
        result.native.colors.append({quint8(integer(rgb[0],255)), quint8(integer(rgb[1],255)), quint8(integer(rgb[2],255))});
    }
    return result;
}
