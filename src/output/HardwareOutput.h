#pragma once
#include "OutputPanel.h"
#include "hid/HidApiDevice.h"
// Requires exactly one control/LED pair of the same PID; ambiguity never falls back to first match.
DeviceIdentity vulcanControlIdentity(const DeviceIdentity& led,const std::vector<HidDescriptor>& devices);
struct OutputProof {
    DeviceIdentity identity;
    DeviceIdentity controlIdentity; // Vulcan control endpoint, reviewed alongside its LED endpoint.
    DeviceProfile profile;
    OutputFrame initial;
    QVector<QByteArray> reports;
};
// Uses memory-only HID recording with the actual module; never opens a physical transport.
QVector<QByteArray> previewOutputReports(const ModuleRegistry& modules, const OutputProof& proof);
bool confirmOutputReports(QWidget* parent, const QVector<OutputProof>& proofs);
QVector<OutputConnection> authorizeHardwareOutput(QWidget* parent, const QVector<DeviceProfile>& profiles,
    const QVector<OutputFrame>& frames, const QString& moduleDirectory);
