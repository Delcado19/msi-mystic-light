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

    // Native mode only; false without sending when the channel is not in Native mode.
    virtual bool setNativeMode(const std::string& channelName, const NativeModeSettings& settings) = 0;

    // Canvas mode only; false without sending when the channel is not in Canvas mode.
    virtual bool pushCanvasColors(const std::string& channelName, const std::vector<MsiColor>& colors) = 0;
};
