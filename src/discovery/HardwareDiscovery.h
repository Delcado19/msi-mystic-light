#pragma once

#include "hid/HidApiDevice.h"
#include "modules/DeviceProfile.h"

struct BoardIdentity {
    QString manufacturer, model;
};

// OS-provided system-board identity only; never USB product strings or probes.
BoardIdentity readBoardIdentity();
DeviceIdentity deviceIdentity(const HidDescriptor& descriptor, const BoardIdentity& board);
struct ProfileCandidate {
    DeviceIdentity device;
    QString profileId;
};
// Preserve all matches: neither enumeration order nor profile specificity selects a device.
QVector<ProfileCandidate> findProfileCandidates(const std::vector<HidDescriptor>& devices,
    const BoardIdentity& board, const QVector<DeviceProfile>& profiles);
