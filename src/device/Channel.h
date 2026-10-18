// Channel model: one controller output chain (e.g. one ARGB header) that
// one or more Components (src/device/Component.h) get appended to, up to
// an LED limit. Pure LED-chain bookkeeping only — no zone-index or
// Gen1Adv-area mapping, which is MsiMotherboardDriver's job, not this
// class's.
#pragma once

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "device/Component.h"
#include "protocol/MsiProtocol.h"

class Channel {
public:
    Channel(std::string name, int ledLimit);

    const std::string& name() const;
    int ledLimit() const;

    // Appends component to the end of this channel's LED chain. Returns
    // false (no-op, channel unchanged) if totalLedCount() + component.ledCount
    // would exceed ledLimit().
    bool addComponent(const Component& component);

    void clearComponents();
    int totalLedCount() const;
    const std::vector<MsiColor>& colors() const;

    // No-op if index is out of [0, totalLedCount()) range.
    void setColor(int index, MsiColor color);

    // Global chain index -> the owning component's declared [x,y], or
    // std::nullopt if index is out of range.
    std::optional<std::pair<int,int>> ledPosition(int index) const;

private:
    std::string name_;
    int ledLimit_ = 0;
    std::vector<Component> components_;
    std::vector<MsiColor> colors_;
};
