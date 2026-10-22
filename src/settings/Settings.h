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
#include "effects/CustomGradientEffect.h"

#include "canvas/Canvas.h"
#include "device/MsiMotherboardDriver.h"
#include "protocol/MsiProtocol.h"

struct ChannelSettings {
    std::string channelName;
    bool layoutVisible=true; // Presentation only; hiding never disables physical lighting.
    ChannelMode mode = ChannelMode::Native;
    CanvasSource source = CanvasSource::Independent; // meaningful only when mode == Canvas
    CanvasPlacement placement{};                      // meaningful only when source == GlobalFollow
    std::string effectName;            // e.g. "StaticColor", "RainbowWave"; empty if mode == Native
    // Meaningful only for effects that actually read a color parameter --
    // StaticColor, Breathing, Comet and Marquee (see LightingStudioWindow.cpp's
    // buildEffect()/effectUsesColor()); ignored by RainbowWave, RadialRainbow,
    // Spiral and SwirlCircles. (Task 23 correction: this comment previously
    // said "meaningful only if effectName == StaticColor", which missed the
    // other three color-taking effects.)
    MsiColor staticColor;
    // Header topology is configurable; peripheral geometry and wire calibration
    // preserve device capacity. Explicitly empty configured headers output black.
    std::vector<Component> components;
    bool componentsConfigured=false; // Distinguishes deliberately empty outputs from legacy defaults.
    bool hasCustomComponents() const {return componentsConfigured || !components.empty();}
    std::map<int, MsiColor> ledOverrides;
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
    std::vector<CustomEffectDefinition> customEffects;
    std::vector<Component> hardwareCatalog;
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

    // Profile CRUD (Task 23: the UI previously had no way to create, rename,
    // or delete a profile at all -- only switch between ones already present
    // in a hand-edited settings file). Kept on Settings rather than inline in
    // LightingStudioWindow so this logic is directly unit-testable in
    // test_settings.cpp, matching this class's existing coverage style.
    //
    // Appends a new, empty profile named `name` (no channelSettings yet --
    // callers such as LightingStudioWindow are expected to fill in every
    // known channel's defaults afterward, e.g. via its own
    // ensureProfileAndEntries()). Does NOT change activeProfileName. False,
    // no-op, if `name` is empty or a profile with that exact name already
    // exists (profiles are looked up by name elsewhere -- activeProfile() --
    // so two profiles sharing a name would make the second unreachable).
    bool addProfile(const std::string& name);

    // Renames the CURRENTLY ACTIVE profile (per activeProfile()'s own
    // stale-name-falls-back-to-first contract -- not necessarily whatever
    // profile is literally named activeProfileName) to `newName`, and
    // updates activeProfileName to match. False, no-op, if profiles is
    // empty, `newName` is empty, or `newName` collides with a DIFFERENT
    // existing profile (renaming a profile to its own current name is a
    // harmless no-op success, not a collision).
    bool renameActiveProfile(const std::string& newName);

    // Removes the currently active profile and makes a neighbor (the new
    // profiles.front()) active. False, no-op, if profiles has one or zero
    // entries -- activeProfile()'s own contract assumes profiles is never
    // empty elsewhere in this codebase, so this refuses to produce an empty
    // list.
    bool removeActiveProfile();
};
