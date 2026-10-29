// SPDX-License-Identifier: GPL-2.0-or-later
//
// Driver module for ASRock Polychrome USB motherboards. Protocol derived
// from OpenRGB (GPL-2.0-or-later); see protocol/AsrockPolychromeProtocol.h
// for the exact upstream source, cross-check sources and the scope this
// reimplementation deliberately narrows (no RGSwap, no Direct/per-LED mode).
#include "modules/IDriverModule.h"
#include "protocol/AsrockPolychromeProtocol.h"

#include <QMap>
#include <QObject>
#include <QSet>
#include <array>

namespace {
const QMap<QString, AsrockPolychromeMode> modes{
    {"off", AsrockPolychromeMode::Off}, {"static", AsrockPolychromeMode::Static},
    {"breathing", AsrockPolychromeMode::Breathing}, {"strobe", AsrockPolychromeMode::Strobe},
    {"spectrum-cycle", AsrockPolychromeMode::SpectrumCycle}, {"random", AsrockPolychromeMode::Random},
    {"music", AsrockPolychromeMode::Music}, {"wave", AsrockPolychromeMode::Wave},
    {"spring", AsrockPolychromeMode::Spring}, {"stack", AsrockPolychromeMode::Stack},
    {"cram", AsrockPolychromeMode::Cram}, {"scan", AsrockPolychromeMode::Scan},
    {"neon", AsrockPolychromeMode::Neon}, {"water", AsrockPolychromeMode::Water},
    {"rainbow", AsrockPolychromeMode::Rainbow}
};

// Three speed levels, matching the existing msi.gen1 convention elsewhere in
// this codebase. Upstream's raw range is 0x00 (fastest) .. 0xFF (slowest).
std::uint8_t mapSpeed(int speed)
{
    switch (speed) {
    case 0: return 0x00;
    case 2: return 0xFF;
    default: return 0xE0; // POLYCHROME_USB_SPEED_DEFAULT
    }
}

class AsrockPolychromeDriver final : public IDriver {
public:
    AsrockPolychromeDriver(IHidDevice& transport, const DeviceProfile& profile) : transport_(transport)
    {
        for (const auto& channel : profile.channels) channels_.insert(channel.id, channel);
    }

    bool setNativeMode(const QString& channelId, const NativeEffect& effect) override
    {
        const auto channel = channels_.constFind(channelId);
        if (!transport_.isOpen() || channel == channels_.cend() ||
            !channel->nativeModes.contains(effect.id) || !modes.contains(effect.id) ||
            effect.colors.size() != 1 || effect.speed < 0 || effect.speed > 2) return false;
        const int zone = channel->address.value("zone").toInt();
        if (zone < 0 || zone >= ASROCK_POLYCHROME_ZONE_COUNT) return false;
        const auto mode = modes.value(effect.id);
        const MsiColor color{effect.colors[0].r, effect.colors[0].g, effect.colors[0].b};
        const auto speed = mapSpeed(effect.speed);

        // Upstream's own doc comment says this hardware "will save with each
        // update" (no software-controlled volatile/persist distinction like
        // MSI's trailing store byte). Never resend unchanged state, to limit
        // flash wear from a continuously-running software effect.
        auto& current = state_[zone];
        if (current.applied && current.mode == mode && current.speed == speed &&
            current.color.r == color.r && current.color.g == color.g && current.color.b == color.b) return true;

        const auto report = BuildAsrockPolychromeZoneReport(
            static_cast<AsrockPolychromeZone>(zone), mode, speed, color);
        if (!transport_.writeOutputReport(report.data(), report.size())) return false;
        drainAck();
        current = {mode, speed, color, true};
        return true;
    }

    // Direct/per-LED Chroma streaming is not implemented; see
    // protocol/AsrockPolychromeProtocol.h for why.
    bool pushColors(const QString&, const QVector<RgbColor>&) override { return false; }

private:
    void drainAck()
    {
        std::uint8_t ack[64];
        transport_.readInputReport(ack, sizeof(ack)); // Best-effort; upstream never inspects this either.
    }

    struct ZoneState {
        AsrockPolychromeMode mode{};
        std::uint8_t speed = 0;
        MsiColor color{};
        bool applied = false;
    };

    IHidDevice& transport_;
    QMap<QString, ChannelProfile> channels_;
    std::array<ZoneState, ASROCK_POLYCHROME_ZONE_COUNT> state_{};
};
}

class AsrockPolychromeModule final : public QObject, public IDriverModule {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DRIVER_MODULE_IID FILE "module.json")
    Q_INTERFACES(IDriverModule)
public:
    bool acceptsProfile(const DeviceProfile& input, QString* error) const override
    {
        const auto reject = [error](const QString& reason) { if (error) *error = reason; return false; };
        if (error) error->clear();
        if (input.driverId != "asrock.polychrome-usb" || input.channels.isEmpty() ||
            input.match.value("vendorId").toInt() != 0x26ce ||
            input.match.value("productId").toInt() != 0x01a2)
            return reject("ASRock Polychrome requires an explicit vendor/product identity");
        QSet<int> zones;
        QSet<QString> ids;
        for (const auto& channel : input.channels) {
            const auto value = channel.address.value("zone");
            const int zone = value.toInt(-1);
            if (!value.isDouble() || value.toDouble() != zone || zone < 0 || zone >= ASROCK_POLYCHROME_ZONE_COUNT ||
                zones.contains(zone) || channel.id.isEmpty() || ids.contains(channel.id) ||
                channel.direct || channel.nativeModes.isEmpty() || channel.maxLeds < 1)
                return reject("Invalid or duplicate ASRock Polychrome channel/zone");
            zones.insert(zone);
            ids.insert(channel.id);
            for (const auto& mode : channel.nativeModes)
                if (!modes.contains(mode)) return reject("Unsupported ASRock Polychrome native mode: " + mode);
        }
        return true;
    }

    std::unique_ptr<IDriver> create(IHidDevice& transport, const DeviceProfile& profile) override
    {
        if (!acceptsProfile(profile, nullptr)) return {};
        return std::make_unique<AsrockPolychromeDriver>(transport, profile);
    }
};

#include "AsrockPolychromeModule.moc"
