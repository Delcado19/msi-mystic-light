// Per-channel user configuration, persisted to a local JSON file so the app
// can restore the last-applied config on launch (v1 skeleton plan goal).
// Consumed/produced by the UI layer (Task 12); ChannelMode/NativeModeSettings
// come from Task 6 (device/MsiMotherboardDriver.h), MsiColor from Task 2
// (protocol/MsiProtocol.h).
#pragma once

#include <string>
#include <vector>

#include "device/MsiMotherboardDriver.h"
#include "protocol/MsiProtocol.h"

struct ChannelSettings {
    std::string channelName;
    ChannelMode mode = ChannelMode::Native;
    std::string effectName;            // e.g. "StaticColor", "RainbowWave"; empty if mode == Native
    MsiColor staticColor;              // meaningful only if effectName == "StaticColor"
    NativeModeSettings nativeSettings; // meaningful only if mode == Native

    bool operator==(const ChannelSettings&) const; // field-by-field, for tests
};

class Settings {
public:
    std::vector<ChannelSettings> channelSettings;

    bool saveToFile(const std::string& path) const; // false on I/O failure
    // False if the file doesn't exist or fails to parse; channelSettings
    // is left UNCHANGED in that case (caller keeps whatever defaults it
    // already had). Review Focus: settings file missing or corrupted on
    // first run or after a manual edit must never crash the app, since
    // "restores last-applied config on launch" is a stated goal — see
    // Settings.cpp for how individual malformed channel entries and
    // missing fields are tolerated rather than rejecting the whole file.
    bool loadFromFile(const std::string& path);
};
