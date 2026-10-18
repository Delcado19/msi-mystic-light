#include "canvas/Canvas.h"

#include <utility> // std::move (currently pulled in transitively; make it explicit)

Canvas::Canvas(MsiMotherboardDriver& driver, std::map<std::string, Channel>& channels)
    : driver_(driver), channels_(channels) {}

void Canvas::setAssignment(const std::string& channelName, ChannelAssignment assignment) {
    bool isNative = assignment.mode == ChannelMode::Native;
    assignments_[channelName] = std::move(assignment);
    // Native-mode settings are a device-side effect: they only need sending
    // once, on the tick right after assignment. Mark pending here; tick()
    // clears it after the first send attempt.
    nativeModePending_[channelName] = isNative;
}

void Canvas::tick(int64_t elapsedMs) {
    for (auto& [channelName, assignment] : assignments_) {
        auto channelIt = channels_.find(channelName);
        if (channelIt == channels_.end()) {
            continue; // unknown channel: nothing to render, skip it
        }
        Channel& channel = channelIt->second;

        if (assignment.mode == ChannelMode::Canvas) {
            if (!assignment.effect) {
                continue; // no effect assigned yet: nothing to sample
            }
            const auto& colors = channel.colors();
            for (int i = 0; i < static_cast<int>(colors.size()); ++i) {
                auto pos = channel.ledPosition(i);
                if (!pos) {
                    continue;
                }
                MsiColor color = assignment.effect->colorAt(pos->first, pos->second, elapsedMs);
                channel.setColor(i, color);
            }
            // Driver call may fail (e.g. board disconnected); per the Review
            // Focus item, that must not abort the rest of this frame, so the
            // return value is intentionally ignored and the loop continues.
            driver_.pushCanvasColors(channelName, channel.colors());
        } else {
            if (nativeModePending_[channelName]) {
                driver_.setNativeMode(channelName, assignment.nativeSettings);
                nativeModePending_[channelName] = false;
            }
        }
    }
}
