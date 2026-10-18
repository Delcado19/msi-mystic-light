// Canvas: the per-frame render-loop dispatcher tying together IEffect
// (Task 7), Channel (Task 5) and MsiMotherboardDriver (Task 6). For each
// assigned channel, either samples its effect per-LED and pushes the result
// (Canvas mode) or sends its native-mode settings once, only on the first
// tick after assignment (Native mode) -- never every tick, since native
// mode is a device-side effect the board keeps running on its own.
#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include "device/Channel.h"
#include "device/MsiMotherboardDriver.h"
#include "effects/IEffect.h"

struct ChannelAssignment {
    ChannelMode mode = ChannelMode::Canvas;
    std::shared_ptr<IEffect> effect;        // used when mode == Canvas
    NativeModeSettings nativeSettings;      // used when mode == Native
};

class Canvas {
public:
    Canvas(MsiMotherboardDriver& driver, std::map<std::string, Channel>& channels);

    // Sets/replaces the assignment for a channel. Marks the channel's
    // native-mode settings as pending-send (see tick()) if mode == Native.
    void setAssignment(const std::string& channelName, ChannelAssignment assignment);

    // One render tick at elapsedMs since some external reference time:
    // - Canvas-mode channels: for every LED index in channel.colors(),
    //   looks up ledPosition(index), samples assignment.effect->colorAt(x,y,elapsedMs),
    //   writes it via channel.setColor(index, ...), then calls
    //   driver.pushCanvasColors(channelName, channel.colors()) once per channel.
    // - Native-mode channels: calls driver.setNativeMode(channelName, assignment.nativeSettings)
    //   ONLY if that channel is still marked pending-send (i.e. only the
    //   first tick after setAssignment() last touched it) -- not every tick.
    // Continues to the next channel if any single channel's driver call
    // returns false; never throws. (Review Focus: a HID send failing
    // mid-session must not abort the rest of the frame.)
    void tick(int64_t elapsedMs);

private:
    MsiMotherboardDriver& driver_;
    std::map<std::string, Channel>& channels_;
    std::map<std::string, ChannelAssignment> assignments_;
    std::map<std::string, bool> nativeModePending_;
};
