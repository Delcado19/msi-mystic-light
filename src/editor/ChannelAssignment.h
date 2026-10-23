#pragma once
#include "modules/IDriverModule.h"
#include <QJsonObject>
#include <QUuid>

struct ChannelAssignment {
    QUuid id = QUuid::createUuid();
    QString endpoint, profileId, channelId;
    bool software = false; // Native channels never enter the per-frame sampling path.
    QVector<QUuid> placements; // Physical chain order, not layout z-order.
    // shortcut: current drivers use 1..4 colors, brightness 0..5 and speed 0..2; add capability ranges for a different driver.
    NativeEffect native;
};
struct LightingState;
const DeviceProfile& assignmentProfile(const QVector<DeviceProfile>& profiles, const QString& id);
const ChannelProfile& assignmentChannel(const DeviceProfile& profile, const QString& id);
void validateChannelAssignments(const LightingState& state, const QVector<DeviceProfile>& profiles);
QJsonObject channelAssignmentJson(const ChannelAssignment& assignment);
ChannelAssignment parseChannelAssignment(const QJsonObject& json);
