#pragma once
#include "discovery/HardwareDiscovery.h"
#include "hid/IHidDevice.h"
#include <memory>
// Default is passive PCI/cached RAM inventory. GUI scans may explicitly probe ENE identity registers.
QVector<ProfileCandidate> scanAuxiliaryHardware(const QVector<DeviceProfile>& profiles,bool probeRam=false);
std::unique_ptr<IHidDevice> openAuxiliaryTransport(const DeviceIdentity& identity);
