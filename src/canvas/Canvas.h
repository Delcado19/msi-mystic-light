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

// A channel's offset within the shared canvas-wide coordinate space.
// Meaningful only when CanvasSource == GlobalFollow.
struct CanvasPlacement { double x = 0.0, y = 0.0; };

// Pixel size of the box a GlobalFollow channel samples the global effect
// through. Single source of truth for two things that must stay in
// lockstep: the region Canvas::tick() actually samples (see its GlobalFollow
// branch) and the region LightingCanvasWidget draws on screen for that
// channel (LightingCanvasWidget::kBoxWidth/kBoxHeight alias these). Before
// Task 22's geometry fix, the drawn box was a fixed 70x40 but the *sampled*
// region was each channel's raw local-coordinate extent (e.g. 120px wide for
// a motherboard JARGB channel's generic placeholder) -- so a wide channel's
// hardware LEDs were driven by pixels far outside what was drawn/highlighted
// on screen. Canvas::tick() now scales every channel's own local-coordinate
// range into this fixed box (see computeLocalExtent/scaleIntoBox in
// Canvas.cpp), so a 120-LED flat line and e.g. the Roccat Vulcan's 24x6 grid
// both sample proportionally from this same box size, generically, with no
// per-device-name special-casing.
constexpr int kGlobalFollowBoxWidth = 70;
constexpr int kGlobalFollowBoxHeight = 40;

struct ChannelAssignment {
    ChannelMode mode = ChannelMode::Canvas;
    CanvasSource source = CanvasSource::Independent; // meaningful only when mode == Canvas
    std::shared_ptr<IEffect> effect;        // used when mode == Canvas && source == Independent
    NativeModeSettings nativeSettings;      // used when mode == Native
    CanvasPlacement placement{};             // this channel's offset in global canvas space; used when source == GlobalFollow
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

    // One render tick at elapsedMs since some external reference time:
    // - Canvas-mode, source == Independent: for every LED index in
    //   channel.colors(), looks up ledPosition(index), samples
    //   assignment.effect->colorAt(x,y,elapsedMs) (skipping the channel this
    //   frame if assignment.effect is null), writes it via channel.setColor,
    //   then pushes via the channel's driver.
    // - Canvas-mode, source == GlobalFollow: same, but samples globalEffect_
    //   at (placement.x + scaledX, placement.y + scaledY), where scaledX/Y
    //   is this channel's local (x,y) rescaled from its own actual
    //   local-coordinate extent (derived from Channel::ledPosition across
    //   all its LEDs -- see computeLocalExtent() in Canvas.cpp) into
    //   [0, kGlobalFollowBoxWidth) / [0, kGlobalFollowBoxHeight) (Task 22
    //   geometry fix: this is the same box LightingCanvasWidget draws, so
    //   what's sampled now always matches what's drawn/highlighted).
    //   Skips the channel this frame if globalEffect_ is null, same as
    //   before.
    // - Native-mode channels: calls driver.setNativeMode(channelName, assignment.nativeSettings)
    //   ONLY if that channel is still marked pending-send (i.e. only the
    //   first tick after setAssignment() last touched it) -- not every tick.
    // Continues to the next channel if any single channel's driver call
    // returns false, or if no driver is routed for it; never throws. (Review
    // Focus: a HID send failing mid-session must not abort the rest of the
    // frame.)
    void tick(int64_t elapsedMs);

private:
    std::map<std::string, IDriver*> driversByChannel_;
    std::map<std::string, Channel>& channels_;
    std::map<std::string, ChannelAssignment> assignments_;
    std::map<std::string, bool> nativeModePending_;
    std::shared_ptr<IEffect> globalEffect_;
};
