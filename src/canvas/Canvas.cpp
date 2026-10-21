#include "canvas/LayoutGeometry.h"
#include "canvas/Canvas.h"

#include <algorithm> // std::min/std::max (computeLocalExtent)
#include <utility> // std::move (currently pulled in transitively; make it explicit)

Canvas::Canvas(std::map<std::string, IDriver*> driversByChannel, std::map<std::string, Channel>& channels)
    : driversByChannel_(std::move(driversByChannel)), channels_(channels) {}

void Canvas::setAssignment(const std::string& channelName, ChannelAssignment assignment) {
    bool isNative = assignment.mode == ChannelMode::Native;
    assignments_[channelName] = std::move(assignment);
    // Native-mode settings are a device-side effect: they only need sending
    // once, on the tick right after assignment. Mark pending here; tick()
    // clears it after the first send attempt.
    nativeModePending_[channelName] = isNative;
}

void Canvas::setGlobalEffect(std::shared_ptr<IEffect> effect) {
    globalEffect_ = std::move(effect);
}

void Canvas::tick(int64_t elapsedMs) {
    for (auto& [channelName, assignment] : assignments_) {
        auto channelIt = channels_.find(channelName);
        if (channelIt == channels_.end()) {
            continue; // unknown channel: nothing to render, skip it
        }
        Channel& channel = channelIt->second;

        auto driverIt = driversByChannel_.find(channelName);
        if (driverIt == driversByChannel_.end() || !driverIt->second) {
            continue; // no driver routed for this channel: nothing to push to
        }
        IDriver& driver = *driverIt->second;

        if (assignment.mode == ChannelMode::Canvas) {
            // Independent samples this channel's own effect; GlobalFollow
            // samples the one shared canvas-wide effect instead. Either way,
            // a missing effect just skips the channel for this frame rather
            // than crashing -- matches v1's existing null-effect tolerance,
            // extended to cover "GlobalFollow set but no global effect yet"
            // (Review Focus item for this task).
            IEffect* effect = assignment.source == CanvasSource::Independent
                ? assignment.effect.get()
                : globalEffect_.get();
            if (!effect && channel.totalLedCount()>0) {
                continue;
            }
            const auto& colors = channel.colors();
            // Recompute from active geometry, including edited groups and wire
            // calibration. Both previews call the same transform below.
            const auto extent=layoutBounds(channel.components());
            for (int i = 0; i < static_cast<int>(colors.size()); ++i) {
                auto pos = channel.ledPosition(i);
                if (!pos) {
                    // Unpopulated Vulcan slots must remain zero, as in OpenRGB.
                    // Padding slots are not LEDs and must not inherit the origin color.
                    channel.setColor(i,{});continue;
                }
                const auto [x,y]=layoutSamplePoint(*pos,extent,assignment.placement,assignment.source==CanvasSource::GlobalFollow);
                int offset=0,local=i,localCount=channel.totalLedCount();
                for(const auto& component:channel.components()){if(i<offset+component.ledCount){local=i-offset;localCount=component.ledCount;for(int logical=0;logical<component.ledCount;++logical)if(component.ledMapping[logical]==local){local=component.effectAddress(logical);break;}break;}offset+=component.ledCount;}
                // GlobalFollow always samples the visible spatial field; an LED-path
                // recipe must not make placement irrelevant for shared lighting.
                MsiColor color = assignment.source==CanvasSource::GlobalFollow?effect->colorAt(x,y,elapsedMs)
                    :effect->colorForLed(x,y,elapsedMs,offset+local,channel.totalLedCount(),local,localCount);
                channel.setColor(i, color);
            }
            // Driver call may fail (e.g. board disconnected); per the Review
            // Focus item, that must not abort the rest of this frame, so the
            // return value is intentionally ignored and the loop continues.
            for (const auto& [index, color] : assignment.ledOverrides) if(assignment.rawWireOverride || channel.ledPosition(index))channel.setColor(index, color);
            driver.pushCanvasColors(channelName, channel.colors());
        } else {
            if (nativeModePending_[channelName]) {
                driver.setNativeMode(channelName, assignment.nativeSettings);
                nativeModePending_[channelName] = false;
            }
        }
    }
}
