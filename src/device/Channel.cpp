#include "device/Channel.h"

Channel::Channel(std::string name, int ledLimit)
    : name_(std::move(name)), ledLimit_(ledLimit)
{
}

const std::string& Channel::name() const
{
    return name_;
}

int Channel::ledLimit() const
{
    return ledLimit_;
}

bool Channel::addComponent(const Component& component)
{
    if (totalLedCount() + component.ledCount > ledLimit_) {
        return false;
    }
    components_.push_back(component);
    colors_.resize(colors_.size() + component.ledCount);
    return true;
}

void Channel::clearComponents()
{
    components_.clear();
    colors_.clear();
}

int Channel::totalLedCount() const
{
    return static_cast<int>(colors_.size());
}

const std::vector<MsiColor>& Channel::colors() const
{
    return colors_;
}

void Channel::setColor(int index, MsiColor color)
{
    if (index < 0 || index >= totalLedCount()) {
        return;
    }
    colors_[index] = color;
}

std::optional<std::pair<int,int>> Channel::ledPosition(int index) const
{
    if (index < 0 || index >= totalLedCount()) {
        return std::nullopt;
    }
    // Walk the chain to find which component owns this global index, then
    // translate to that component's own local LED index.
    int offset = 0;
    for (const Component& c : components_) {
        if (index < offset + c.ledCount) {
            return c.ledCoordinates[index - offset];
        }
        offset += c.ledCount;
    }
    return std::nullopt; // unreachable: totalLedCount() bound above guarantees a match
}
