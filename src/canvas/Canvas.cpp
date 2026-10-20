#include "canvas/Canvas.h"

#include <algorithm> // std::min/std::max (computeLocalExtent)
#include <utility> // std::move (currently pulled in transitively; make it explicit)

namespace {

// A channel's local LED-coordinate bounding box, derived from its actual
// attached Component data (Channel::ledPosition, which resolves to each
// owning Component's own ledCoordinates -- see device/Component.h). Generic
// over any channel shape: a flat 1-row line (Component::makeGenericPlaceholder,
// used by the motherboard's 120-LED channels), a 2D grid (e.g. the Roccat
// Vulcan's real 24x6 key layout), or anything else a Component JSON
// declares -- no per-device-name special-casing (Task 22 geometry fix).
struct LocalExtent { int minX = 0, maxX = 0, minY = 0, maxY = 0; };

LocalExtent computeLocalExtent(const Channel& channel)
{
    LocalExtent extent;
    bool first = true;
    for (int i = 0; i < channel.totalLedCount(); ++i) {
        auto pos = channel.ledPosition(i);
        if (!pos) {
            continue;
        }
        if (first) {
            extent.minX = extent.maxX = pos->first;
            extent.minY = extent.maxY = pos->second;
            first = false;
        } else {
            extent.minX = std::min(extent.minX, pos->first);
            extent.maxX = std::max(extent.maxX, pos->first);
            extent.minY = std::min(extent.minY, pos->second);
            extent.maxY = std::max(extent.maxY, pos->second);
        }
    }
    return extent;
}

// Rescales one local coordinate from this channel's own [lo, hi] extent
// into [0, boxSize - 1], so the channel's extreme LEDs land exactly on the
// shared GlobalFollow box's edges regardless of its raw coordinate range.
// hi == lo (e.g. a single-LED channel, or a flat line's constant axis) maps
// to 0 rather than dividing by zero.
int scaleIntoBox(int local, int lo, int hi, int boxSize)
{
    if (hi <= lo) {
        return 0;
    }
    return static_cast<int>(static_cast<int64_t>(local - lo) * (boxSize - 1) / (hi - lo));
}

} // namespace

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
            // GlobalFollow's local-coordinate extent, recomputed fresh each
            // tick rather than cached: components can be reassigned to a
            // channel between ticks (Channel::addComponent/clearComponents),
            // and totalLedCount() is small enough (<= a few hundred) that
            // this second O(n) pass is negligible next to the sampling loop
            // below. Unused (default-constructed) for Independent, which
            // never scales -- it samples its own effect at its raw local
            // (x,y), same as v1.
            const LocalExtent extent = assignment.source == CanvasSource::GlobalFollow
                ? computeLocalExtent(channel)
                : LocalExtent{};
            for (int i = 0; i < static_cast<int>(colors.size()); ++i) {
                auto pos = channel.ledPosition(i);
                if (!pos) {
                    continue;
                }
                int x = pos->first;
                int y = pos->second;
                if (assignment.source == CanvasSource::GlobalFollow) {
                    // Task 22 geometry fix: rescale this channel's own local
                    // (x,y) from its real local-coordinate extent (whatever
                    // shape its attached Component(s) declare) into the
                    // fixed GlobalFollow box (kGlobalFollowBoxWidth/Height,
                    // Canvas.h) BEFORE adding the placement offset. Previously
                    // the raw local coordinate was added straight to
                    // placement, so e.g. a 120-wide motherboard channel
                    // sampled a 120px line starting at the box's corner --
                    // far past the fixed-size box LightingCanvasWidget drew
                    // on screen for it. Placement is double (UI may drag to
                    // fractional positions); truncate only once, after
                    // adding the already-integer scaled offset.
                    const int scaledX = scaleIntoBox(x, extent.minX, extent.maxX, kGlobalFollowBoxWidth);
                    const int scaledY = scaleIntoBox(y, extent.minY, extent.maxY, kGlobalFollowBoxHeight);
                    x = static_cast<int>(assignment.placement.x) + scaledX;
                    y = static_cast<int>(assignment.placement.y) + scaledY;
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
