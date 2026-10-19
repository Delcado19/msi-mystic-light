// Per-channel user configuration, persisted to a local JSON file so the app
// can restore the last-applied config on launch (v1 skeleton plan goal).
// Consumed/produced by the UI layer (Task 15); ChannelMode/NativeModeSettings
// come from Task 1 (device/DeviceTypes.h, included transitively via
// MsiMotherboardDriver.h), MsiColor from Task 2 (protocol/MsiProtocol.h).
// CanvasSource/CanvasPlacement come from Task 3 (canvas/Canvas.h) --
// Settings persists the same per-channel canvas placement/follow-mode
// Canvas itself understands at runtime.
#pragma once

#include <string>
#include <vector>

#include "canvas/Canvas.h"
#include "device/MsiMotherboardDriver.h"
#include "protocol/MsiProtocol.h"

struct ChannelSettings {
    std::string channelName;
    ChannelMode mode = ChannelMode::Native;
    CanvasSource source = CanvasSource::Independent; // meaningful only when mode == Canvas
    CanvasPlacement placement{};                      // meaningful only when source == GlobalFollow
    std::string effectName;            // e.g. "StaticColor", "RainbowWave"; empty if mode == Native
    MsiColor staticColor;              // meaningful only if effectName == "StaticColor"
    NativeModeSettings nativeSettings; // meaningful only if mode == Native

    bool operator==(const ChannelSettings&) const; // field-by-field, for tests
};

// Settings for the single canvas-wide effect that GlobalFollow channels
// sample from (Task 3's Canvas::setGlobalEffect). periodMs/speed per effect
// are intentionally not yet configurable here:
// ponytail: hardcode each effect's own default period for v1; add fields
// here when the UI actually exposes a speed control.
struct GlobalEffectSettings {
    std::string effectName;
    MsiColor color;
    int referenceX = 0, referenceY = 0; // meaningful only for effects implementing IPositionableEffect

    bool operator==(const GlobalEffectSettings&) const;
};

// One named, independently switchable lighting configuration: every
// channel's assignment plus the shared global effect. Settings can hold
// several (Task 15's profile selector), but only one is active at a time.
struct Profile {
    std::string name;
    std::vector<ChannelSettings> channelSettings;
    GlobalEffectSettings globalEffect;

    bool operator==(const Profile&) const;
};

class Settings {
public:
    std::vector<Profile> profiles;
    std::string activeProfileName;

    bool saveToFile(const std::string& path) const; // false on I/O failure
    // False if the file doesn't exist or fails to parse; profiles/
    // activeProfileName are left UNCHANGED in that case (caller keeps
    // whatever defaults it already had). Review Focus: settings file
    // missing or corrupted on first run or after a manual edit must never
    // crash the app, since "restores last-applied config on launch" is a
    // stated goal -- see Settings.cpp for how individual malformed channel
    // entries and missing fields are tolerated rather than rejecting the
    // whole file, and for the v1 "{"channels": [...]}" -> single-profile
    // migration that keeps existing users' settings.json working unchanged.
    bool loadFromFile(const std::string& path);

    // Convenience accessor used by main.cpp/UI: returns a pointer to the
    // profile named activeProfileName, or to profiles.front() if that name
    // isn't found -- e.g. a profile was renamed/deleted elsewhere and the
    // settings file still names the old one (Review Focus item) -- or
    // nullptr if profiles is empty.
    Profile* activeProfile();
};
