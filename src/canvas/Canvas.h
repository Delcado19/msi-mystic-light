// Canvas: the per-frame render-loop dispatcher tying together IEffect
// (Task 7), Channel (Task 5) and IDriver (Task 1) implementations. For each
// assigned channel, either samples an effect per-LED and pushes the result
// (Canvas mode) or sends its native-mode settings once, only on the first
// tick after assignment (Native mode) -- never every tick, since native
// mode is a device-side effect the board keeps running on its own.
//
// Canvas mode has two color sources (CanvasSource): a channel can sample
// its own Independent effect at its local (x,y), same as v1, or it can
// GlobalFollow one shared canvas-wide effect, sampled at this channel's
// placement offset plus its local (x,y) -- letting several channels act as
// windows onto one continuous effect laid out across the whole canvas
// (Task 3 of the Lighting Studio plan).
#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include "device/Channel.h"
#include "device/IDriver.h"
#include "effects/IEffect.h"

// Where a Canvas-mode channel's per-LED colors come from.
enum class CanvasSource { Independent, GlobalFollow };

// Scene-space footprint; rotation is around the footprint center. Legacy
// profiles retain their 70x40 size until the user scales them.
struct CanvasPlacement {
    double x=0.,y=0.,width=70.,height=40.,rotation=0.;
};
constexpr int kGlobalFollowBoxWidth=70;
constexpr int kGlobalFollowBoxHeight=40;

struct ChannelAssignment {
    ChannelMode mode = ChannelMode::Canvas;
    CanvasSource source = CanvasSource::Independent; // meaningful only when mode == Canvas
    std::shared_ptr<IEffect> effect;        // used when mode == Canvas && source == Independent
    NativeModeSettings nativeSettings;      // used when mode == Native
    std::map<int, MsiColor> ledOverrides; // Explicit per-region colors override the sampled effect.
    CanvasPlacement placement{};             // this channel's offset in global canvas space; used when source == GlobalFollow
    bool rawWireOverride=false; // Temporary calibration may probe unmapped slots. Never persisted.
};

class Canvas {
public:
    // channelName -> the IDriver responsible for it. Several channel names
    // may map to the same IDriver* (e.g. all 6 motherboard channels share
    // one MsiMotherboardDriver instance); each peripheral channel maps to
    // its own driver instance.
    Canvas(std::map<std::string, IDriver*> driversByChannel, std::map<std::string, Channel>& channels);

    // Sets/replaces the assignment for a channel. Marks the channel's
    // native-mode settings as pending-send (see tick()) if mode == Native.
    void setAssignment(const std::string& channelName, ChannelAssignment assignment);

    // Sets the single canvas-wide effect that GlobalFollow channels sample
    // from. Independent channels never read this. A GlobalFollow channel
    // with no global effect set is tolerated (see tick()), since the studio
    // UI may place a channel before the user picks a global effect.
    void setGlobalEffect(std::shared_ptr<IEffect> effect);

    // Samples every mapped address using LayoutGeometry's shared footprint
    // transform, then applies fixed region colors and dispatches the wire frame.
    // Unpopulated slots remain black except during explicit raw calibration.
    // Native effects are sent only once after an assignment change.
    void tick(int64_t elapsedMs);

private:
    std::map<std::string, IDriver*> driversByChannel_;
    std::map<std::string, Channel>& channels_;
    std::map<std::string, ChannelAssignment> assignments_;
    std::map<std::string, bool> nativeModePending_;
    std::shared_ptr<IEffect> globalEffect_;
};
