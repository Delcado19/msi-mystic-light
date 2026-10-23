#pragma once
#include "OutputPanel.h"
struct OutputProof {
    DeviceIdentity identity;
    DeviceProfile profile;
    OutputFrame initial;
    QVector<QByteArray> reports;
};
// Uses memory-only HID recording with the actual module; never opens a physical transport.
QVector<QByteArray> previewOutputReports(const ModuleRegistry& modules, const OutputProof& proof);
bool confirmOutputReports(QWidget* parent, const QVector<OutputProof>& proofs);
QVector<OutputConnection> authorizeHardwareOutput(QWidget* parent, const QVector<DeviceProfile>& profiles,
    const QVector<OutputFrame>& frames, const QString& moduleDirectory);
