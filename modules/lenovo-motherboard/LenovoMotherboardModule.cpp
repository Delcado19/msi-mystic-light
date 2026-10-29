// SPDX-License-Identifier: GPL-2.0-or-later
//
// Driver module for Lenovo motherboard RGB. Protocol derived from OpenRGB
// (GPL-2.0-or-later); see protocol/LenovoMotherboardProtocol.h for the
// exact upstream source.
#include "modules/IDriverModule.h"
#include "protocol/LenovoMotherboardProtocol.h"

#include <QMap>
#include <QObject>
#include <QSet>
#include <algorithm>

namespace {
const QMap<QString, LenovoMotherboardMode> modes{
    {"static", LenovoMotherboardMode::Static}, {"sparkle", LenovoMotherboardMode::Sparkle},
    {"breathing", LenovoMotherboardMode::Breathing}, {"wave", LenovoMotherboardMode::Wave},
    {"specter", LenovoMotherboardMode::Specter}, {"rainbow-wave", LenovoMotherboardMode::RainbowWave},
    {"random", LenovoMotherboardMode::Random}
};

bool zoneFromAddress(int value, LenovoMotherboardZone* zone)
{
    if (value == static_cast<int>(LenovoMotherboardZone::CpuFan)) { *zone = LenovoMotherboardZone::CpuFan; return true; }
    if (value == static_cast<int>(LenovoMotherboardZone::RearFan)) { *zone = LenovoMotherboardZone::RearFan; return true; }
    return false;
}

// Upstream's documented range is 1..4 for both; direction (which end is
// fastest/brightest) is not documented and was not verified against real
// hardware. This linear clamp is a reasonable default, not a verified mapping.
std::uint8_t mapSpeed(int speed) { return static_cast<std::uint8_t>(std::clamp(speed + 1, 1, 4)); }
std::uint8_t mapBrightness(int brightness) { return static_cast<std::uint8_t>(std::clamp(brightness, 1, 4)); }

class LenovoMotherboardDriver final : public IDriver {
public:
    LenovoMotherboardDriver(IHidDevice& transport, const DeviceProfile& profile) : transport_(transport)
    {
        for (const auto& channel : profile.channels) channels_.insert(channel.id, channel);
    }

    bool setNativeMode(const QString& channelId, const NativeEffect& effect) override
    {
        const auto channel = channels_.constFind(channelId);
        LenovoMotherboardZone zone{};
        if (!transport_.isOpen() || channel == channels_.cend() ||
            !channel->nativeModes.contains(effect.id) || !modes.contains(effect.id) ||
            effect.colors.size() != 1 || effect.speed < 0 || effect.speed > 2 ||
            effect.brightness < 0 || effect.brightness > 5 ||
            !zoneFromAddress(channel->address.value("zone").toInt(-1), &zone)) return false;

        const auto reports = BuildLenovoMotherboardReports(
            zone, modes.value(effect.id), mapSpeed(effect.speed), mapBrightness(effect.brightness),
            MsiColor{effect.colors[0].r, effect.colors[0].g, effect.colors[0].b});
        for (const auto& report : reports)
            if (!transport_.sendFeatureReport(report.data(), report.size())) return false;
        return true;
    }

    bool pushColors(const QString&, const QVector<RgbColor>&) override { return false; }

private:
    IHidDevice& transport_;
    QMap<QString, ChannelProfile> channels_;
};
}

class LenovoMotherboardModule final : public QObject, public IDriverModule {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DRIVER_MODULE_IID FILE "module.json")
    Q_INTERFACES(IDriverModule)
public:
    bool acceptsProfile(const DeviceProfile& input, QString* error) const override
    {
        const auto reject = [error](const QString& reason) { if (error) *error = reason; return false; };
        if (error) error->clear();
        if (input.driverId != "lenovo.motherboard" || input.channels.isEmpty() ||
            input.match.value("vendorId").toInt() != 0x17ef ||
            input.match.value("productId").toInt() != 0xc955)
            return reject("Lenovo motherboard requires an explicit vendor/product identity");
        QSet<int> zones;
        QSet<QString> ids;
        for (const auto& channel : input.channels) {
            const auto value = channel.address.value("zone");
            const int zone = value.toInt(-1);
            LenovoMotherboardZone parsedZone{};
            if (!value.isDouble() || value.toDouble() != zone || !zoneFromAddress(zone, &parsedZone) ||
                zones.contains(zone) || channel.id.isEmpty() || ids.contains(channel.id) ||
                channel.direct || channel.nativeModes.isEmpty() || channel.maxLeds < 1)
                return reject("Invalid or duplicate Lenovo motherboard channel/zone");
            zones.insert(zone);
            ids.insert(channel.id);
            for (const auto& mode : channel.nativeModes)
                if (!modes.contains(mode)) return reject("Unsupported Lenovo motherboard native mode: " + mode);
        }
        return true;
    }

    std::unique_ptr<IDriver> create(IHidDevice& transport, const DeviceProfile& profile) override
    {
        if (!acceptsProfile(profile, nullptr)) return {};
        return std::make_unique<LenovoMotherboardDriver>(transport, profile);
    }
};

#include "LenovoMotherboardModule.moc"
