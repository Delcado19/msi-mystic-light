#include "canvas/Canvas.h"

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
            if (!effect) {
                continue;
            }
            const auto& colors = channel.colors();
            for (int i = 0; i < static_cast<int>(colors.size()); ++i) {
                auto pos = channel.ledPosition(i);
                if (!pos) {
                    continue;
                }
                int x = pos->first;
                int y = pos->second;
                if (assignment.source == CanvasSource::GlobalFollow) {
                    // Global canvas space: this channel's placement offset
                    // plus its local LED position. Placement is double (UI
                    // may drag channels to fractional positions), so round
                    // down to int only once it's added to the local
                    // coordinate, per the effect interface's int contract.
                    x = static_cast<int>(assignment.placement.x + x);
                    y = static_cast<int>(assignment.placement.y + y);
                }
                MsiColor color = effect->colorAt(x, y, elapsedMs);
                channel.setColor(i, color);
            }
            // Driver call may fail (e.g. board disconnected); per the Review
            // Focus item, that must not abort the rest of this frame, so the
            // return value is intentionally ignored and the loop continues.
            driver.pushCanvasColors(channelName, channel.colors());
        } else {
            if (nativeModePending_[channelName]) {
                driver.setNativeMode(channelName, assignment.nativeSettings);
                nativeModePending_[channelName] = false;
            }
        }
    }
}
