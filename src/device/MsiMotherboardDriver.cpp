#include "device/MsiMotherboardDriver.h"

#include <optional>

namespace {

// Per-channel zone-index / Gen1Adv-area / LED-limit mapping for this exact
// board (MPG B850 Edge TI WiFi / MS-7E62). This mapping is NOT part of
// Channel's public interface (Channel only knows name+ledLimit) - it is
// private to the driver, which is the sole owner of the board's fixed HID
// topology. Confirmed zone-index assignment per docs/MB_800_HID_PROTOCOL.md:
// JARGB_V2_1=0, JARGB_V2_2=1, JARGB_V2_3=2, EZ Conn=3, IO COVER=4, JRGB1=9.
// JRGB1 has no Gen1Adv area: it is a single non-addressable LED, native-mode
// only, so report 0x51 (per-LED Canvas push) never applies to it.
struct ChannelInfo {
    int zoneIndex;
    std::optional<MsiGen1AdvArea> gen1AdvArea;
    int ledLimit;
};

const std::map<std::string, ChannelInfo>& channelTable()
{
    static const std::map<std::string, ChannelInfo> table = {
        {"JARGB1",      {0, MsiGen1AdvArea::JARGB1, 120}},
        {"JARGB2",      {1, MsiGen1AdvArea::JARGB2, 120}},
        {"JARGB3",      {2, MsiGen1AdvArea::JARGB3, 120}},
        {"EZConnector", {3, MsiGen1AdvArea::JAF,    120}},
        {"IOShield",    {4, MsiGen1AdvArea::JPIPE1, 11}},
        {"JRGB1",       {9, std::nullopt,           1}},
    };
    return table;
}

} // namespace

MsiMotherboardDriver::MsiMotherboardDriver(IHidDevice& hid) : hid_(hid)
{
    // Brief's ruling: default channelModes_ to Native for every channel.
    for (const auto& [name, info] : channelTable()) {
        channelModes_[name] = ChannelMode::Native;
    }
}

std::vector<Channel> MsiMotherboardDriver::createDefaultChannels()
{
    // Fixed, documented order (not the alphabetical std::map order above).
    static const std::vector<std::string> order = {
        "JARGB1", "JARGB2", "JARGB3", "EZConnector", "IOShield", "JRGB1"
    };
    std::vector<Channel> channels;
    channels.reserve(order.size());
    for (const auto& name : order) {
        const int ledLimit = channelTable().at(name).ledLimit;
        channels.emplace_back(name, ledLimit);
        // Bug found during live HW verification: without a Component, Channel::colors()
        // is always empty, so Canvas's report 0x51 silently sends zero LEDs for every
        // motherboard channel (0x00 at offset 6 instead of e.g. 0x78/120) even though
        // the HID write itself succeeds. Every peripheral driver's createDefaultChannel()
        // already does this; this was the one place it was missed. Native mode (0x50)
        // never used colors(), which is why it kept "working" and hid the gap.
        channels.back().addComponent(Component::makeGenericPlaceholder(ledLimit));
    }
    return channels;
}

bool MsiMotherboardDriver::supportsMode(const std::string& channelName, ChannelMode mode) const
{
    auto it = channelTable().find(channelName);
    if (it == channelTable().end()) {
        return false;
    }
    // Canvas mode needs a Gen1Adv area for per-LED addressing; JRGB1 has
    // none (single non-addressable LED), so it is native-mode only. Every
    // known channel supports Native.
    if (mode == ChannelMode::Canvas) {
        return it->second.gen1AdvArea.has_value();
    }
    return true;
}

bool MsiMotherboardDriver::setChannelMode(const std::string& channelName, ChannelMode mode)
{
    if (!supportsMode(channelName, mode)) {
        return false;
    }
    auto it = channelTable().find(channelName); // known to exist: supportsMode() already confirmed it
    channelModes_[channelName] = mode;
    // Live-hardware finding (2026-10-20): report 0x51 ("Gen1_Adv", per-LED
    // color data) is only honored by the firmware for a zone whose 0x50
    // record has lightingMode=Advanced (9) - the report's own name is the
    // literal clue. Canvas::tick() never sends a 0x50 report for a
    // Canvas-mode channel (only pushCanvasColors/0x51), so without this,
    // that zone's lightingMode silently stays at MsiZoneSettings's default
    // (Off) and the board never displays the 0x51 data at all, even though
    // the 0x51 send itself is byte-perfect. Confirmed on two independent,
    // real, MSI-Center-UI-verified zones: JARGB1 (rear exhaust fan) and
    // JARGB2 (3 side fans + 1 bottom fan + an LED strip), both went solid
    // red from a 0x51 push only after this arming send. Steady was tried
    // first and did NOT work, ruling out "any non-Off mode" as the real
    // requirement - Advanced specifically is needed.
    //
    // Only lightingMode is touched: brightness/colorCount/color1-4 are left
    // at the zone's existing values. The live test's own result rules out
    // needing more than that - color1 stayed black/default in both
    // successful tests, yet the real LEDs showed the 0x51-pushed red, not
    // black, so once Advanced is active, 0x51's own per-LED data is what
    // the firmware displays and the 0x50 record's color fields don't matter.
    //
    // No "already armed" flag here -- this send is unconditional on every
    // setChannelMode(Canvas) call, same as RoccatVulcanDriver::setChannelMode's
    // EnableDirect send has no enable-cache either (though the two differ in
    // return-value semantics: Vulcan's setChannelMode propagates the actual
    // sendFeatureReport result, this one always returns true once
    // supportsMode() has accepted the mode). As of main.cpp's Task 20 fix,
    // this is no longer reached on every canvas-drag pixel: main.cpp now
    // gates its own call to setChannelMode() on the channel's *mode* actually
    // changing (cached per channel name, not per profile), so a
    // placement-only drag never re-enters this function at all, and a
    // profile switch only re-enters it for channels whose mode actually
    // differs from whatever was last applied under that same channel name --
    // not unconditionally once per switch. Still no cache needed here
    // either way, since every call that does arrive is a real mode change.
    if (mode == ChannelMode::Canvas && hid_.isOpen()) {
        MsiZoneSettings& zone = zoneState_[it->second.zoneIndex];
        zone.lightingMode = MsiLightingMode::Advanced;
        const std::array<uint8_t, 290> report = BuildReport0x50(zoneState_);
        hid_.sendFeatureReport(report.data(), report.size());
    }
    return true;
}

ChannelMode MsiMotherboardDriver::channelMode(const std::string& channelName) const
{
    auto it = channelModes_.find(channelName);
    return it != channelModes_.end() ? it->second : ChannelMode::Native;
}

bool MsiMotherboardDriver::setNativeMode(const std::string& channelName, const NativeModeSettings& settings)
{
    if (!hid_.isOpen()) {
        return false;
    }
    auto it = channelTable().find(channelName);
    if (it == channelTable().end()) {
        return false;
    }
    if (channelMode(channelName) != ChannelMode::Native) {
        return false;
    }

    // NativeModeSettings mirrors MsiZoneSettings field-for-field (see the
    // header comment on NativeModeSettings), so this is a plain copy.
    MsiZoneSettings& zone = zoneState_[it->second.zoneIndex];
    zone.lightingMode = settings.lightingMode;
    zone.color1 = settings.color1;
    zone.color2 = settings.color2;
    zone.color3 = settings.color3;
    zone.color4 = settings.color4;
    zone.colorCount = settings.colorCount;
    zone.selectAll = settings.selectAll;
    zone.directionInward = settings.directionInward;
    zone.colorSelectionUser = settings.colorSelectionUser;
    zone.brightness = settings.brightness;
    zone.speed = settings.speed;
    zone.cycleNum = settings.cycleNum;

    // zoneState_ holds the last-sent settings for ALL 18 zones, so this
    // send carries every other zone's most recent state forward unchanged.
    const std::array<uint8_t, 290> report = BuildReport0x50(zoneState_);
    return hid_.sendFeatureReport(report.data(), report.size());
}

bool MsiMotherboardDriver::pushCanvasColors(const std::string& channelName, const std::vector<MsiColor>& colors)
{
    if (!hid_.isOpen()) {
        return false;
    }
    auto it = channelTable().find(channelName);
    if (it == channelTable().end()) {
        return false;
    }
    if (channelMode(channelName) != ChannelMode::Canvas) {
        return false;
    }
    // Unreachable in practice: setChannelMode already refuses Canvas for a
    // channel with no Gen1Adv area. Guarded anyway so this method can never
    // dereference an empty optional if that invariant is ever broken.
    if (!it->second.gen1AdvArea.has_value()) {
        return false;
    }
    // Contract addition beyond the brief: BuildReport0x51 throws
    // std::invalid_argument above 240 colors, and the Review Focus item for
    // this driver requires every call to fail gracefully rather than throw
    // or crash. Rejecting anything over this channel's own LED limit (always
    // <=120, so always under the 240 cap) also stops overdriving a
    // short chain, e.g. 11-LED IOShield.
    if (colors.size() > static_cast<size_t>(it->second.ledLimit)) {
        return false;
    }

    const std::array<uint8_t, 727> report = BuildReport0x51(*it->second.gen1AdvArea, colors);
    return hid_.sendFeatureReport(report.data(), report.size());
}
