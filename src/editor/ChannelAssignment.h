#pragma once
#include "modules/IDriverModule.h"
#include <QJsonObject>
#include <QUuid>

struct ChannelAssignment {
    QUuid id = QUuid::createUuid();
    QString endpoint, profileId, channelId;
    bool software = false; // Native channels never enter the per-frame sampling path.
    QVector<QUuid> placements; // Physical chain order; standalone outputs have at most one component.
    // shortcut: current drivers use 1..4 colors, brightness 0..5 and speed 0..2; add capability ranges for a different driver.
    NativeEffect native;
    double timeScale = 1, timeOffset = 0; // Channel clock: clamp(global * scale + offset, 0, 86400); zero scale holds.
};
struct LightingState;
const DeviceProfile& assignmentProfile(const QVector<DeviceProfile>& profiles, const QString& id);
const ChannelProfile& assignmentChannel(const DeviceProfile& profile, const QString& id);
// A componentId identifies an integrated device, not a header accepting a chain.
bool channelHasFixedComponent(const ChannelProfile& channel);
void validateChannelAssignments(const LightingState& state, const QVector<DeviceProfile>& profiles);
QJsonObject channelAssignmentJson(const ChannelAssignment& assignment);
ChannelAssignment parseChannelAssignment(const QJsonObject& json, int schema = 9);
