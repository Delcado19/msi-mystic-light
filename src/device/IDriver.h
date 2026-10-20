// Common interface for device drivers. Canvas talks to drivers only through
// this, so a new peripheral only has to implement these four methods.
#pragma once

#include <string>
#include <vector>

#include "device/DeviceTypes.h"
#include "protocol/MsiProtocol.h"

class IDriver {
public:
    virtual ~IDriver() = default;

    // Same contract as MsiMotherboardDriver: returns false (no-op) for an
    // unknown channel or an unsupported mode for that channel.
    virtual bool setChannelMode(const std::string& channelName, ChannelMode mode) = 0;
    virtual ChannelMode channelMode(const std::string& channelName) const = 0;

    // Non-mutating capability query: true if channelName really supports
    // mode on this hardware -- the exact same decision setChannelMode
    // already makes internally to accept/reject a mode change, but without
    // sending anything, touching HID state, or requiring the device to be
    // open. Returns false for an unknown channel name, mirroring
    // setChannelMode's own unknown-channel contract. Added for Task 23 (post-
    // review fix batch): LightingStudioWindow queries this once per channel
    // at startup (via main.cpp) so its mode combo only ever offers a mode
    // the channel's own driver will actually honor -- see
    // .superpowers/sdd/2026-10-19-lighting-studio/task-23-report.md for why
    // this lives on IDriver rather than being re-derived as magic strings in
    // the UI layer. Each driver's setChannelMode() is expected to call this
    // for its own accept/reject check, so there is exactly one place per
    // driver that knows its mode-support rule, not two.
    virtual bool supportsMode(const std::string& channelName, ChannelMode mode) const = 0;

    // Native mode only; false without sending when the channel is not in Native mode.
    virtual bool setNativeMode(const std::string& channelName, const NativeModeSettings& settings) = 0;

    // Canvas mode only; false without sending when the channel is not in Canvas mode.
    virtual bool pushCanvasColors(const std::string& channelName, const std::vector<MsiColor>& colors) = 0;
};
