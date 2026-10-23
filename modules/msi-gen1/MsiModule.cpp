#include "modules/IDriverModule.h"
#include "protocol/MsiProtocol.h"

#include <QJsonArray>
#include <QObject>
#include <QSet>

namespace {
const QMap<QString, MsiLightingMode> modes{
    {"off", MsiLightingMode::Off}, {"wave", MsiLightingMode::Wave},
    {"steady", MsiLightingMode::Steady}, {"flame", MsiLightingMode::Flame},
    {"breathing", MsiLightingMode::Breathing}, {"color-ring", MsiLightingMode::ColorRing},
    {"lightning", MsiLightingMode::Lightning}, {"recreation", MsiLightingMode::Recreation},
    {"meteor", MsiLightingMode::Meteor}, {"godlike", MsiLightingMode::GodLike}
};

class MsiDriver final : public IDriver {
public:
    MsiDriver(IHidDevice& transport, const DeviceProfile& profile) : transport_(transport)
    {
        for (const auto& channel : profile.channels) channels_.insert(channel.id, channel);
    }

    bool setNativeMode(const QString& channelId, const NativeEffect& effect) override
    {
        const auto channel = channels_.constFind(channelId);
        if (!transport_.isOpen() || channel == channels_.cend() ||
            !channel->nativeModes.contains(effect.id) || !modes.contains(effect.id) ||
            effect.colors.isEmpty() || effect.colors.size() > 4 ||
            effect.brightness < 0 || effect.brightness > 5 || effect.speed < 0 || effect.speed > 2) return false;
        const int zone = channel->address.value("zone").toInt();
        auto next = state_;
        auto& settings = next[zone];
        settings = {};
        settings.lightingMode = modes.value(effect.id);
        settings.colorCount = static_cast<std::uint8_t>(effect.colors.size());
        MsiColor* colors[]{&settings.color1, &settings.color2, &settings.color3, &settings.color4};
        for (int i = 0; i < effect.colors.size(); ++i)
            *colors[i] = {effect.colors[i].r, effect.colors[i].g, effect.colors[i].b};
        settings.brightness = static_cast<std::uint8_t>(effect.brightness);
        settings.speed = static_cast<std::uint8_t>(effect.speed);
        settings.colorSelectionUser = true;
        settings.directionInward = effect.inward;
        if (!apply(next)) return false;
        armed_.remove(channelId);
        return true;
    }

    bool pushColors(const QString& channelId, const QVector<RgbColor>& colors) override
    {
        const auto channel = channels_.constFind(channelId);
        if (!transport_.isOpen()) { armed_.clear(); return false; }
        if (channel == channels_.cend() || !channel->direct || colors.isEmpty() ||
            colors.size() > channel->maxLeds ||
            (channel->fixedLeds && colors.size() != channel->fixedLeds)) return false;
        if (!armed_.contains(channelId)) {
            auto next = state_;
            next[channel->address.value("zone").toInt()] = {};
            next[channel->address.value("zone").toInt()].lightingMode = MsiLightingMode::Advanced;
            // Historical Lighting Studio hardware finding (2026-10-20):
            // 0x51 needs the matching 0x50 zone armed to Advanced first.
            if (!apply(next)) return false;
            armed_.insert(channelId);
        }
        std::vector<MsiColor> wireColors;
        wireColors.reserve(colors.size());
        for (const auto color : colors) wireColors.push_back({color.r, color.g, color.b});
        const auto report = BuildReport0x51(
            static_cast<MsiGen1AdvArea>(channel->address.value("area").toInt()), wireColors);
        if (!transport_.sendFeatureReport(report.data(), report.size())) {
            armed_.remove(channelId);
            return false;
        }
        return true;
    }

private:
    bool apply(const std::array<MsiZoneSettings, 18>& next)
    {
        // The driver owns the board for one connection session. The initial
        // full report starts unconfigured zones at Off; subsequent sends retain
        // all successfully applied zone state and never persist to flash.
        const auto report = BuildReport0x50(next, false);
        if (!transport_.sendFeatureReport(report.data(), report.size())) {
            // Failed OS writes leave hardware state uncertain; rearm direct
            // channels on their next update instead of trusting the cache.
            armed_.clear();
            return false;
        }
        state_ = next;
        return true;
    }
    IHidDevice& transport_;
    QMap<QString, ChannelProfile> channels_;
    QSet<QString> armed_;
    std::array<MsiZoneSettings, 18> state_{};
};
}

class MsiModule final : public QObject, public IDriverModule {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DRIVER_MODULE_IID FILE "module.json")
    Q_INTERFACES(IDriverModule)
public:
    bool acceptsProfile(const DeviceProfile& input, QString* error) const override
    {
        const auto reject = [error](const QString& reason) { if (error) *error = reason; return false; };
        if (error) error->clear();
        if (input.driverId != "msi.gen1" || input.channels.isEmpty() ||
            input.match.value("vendorId").toInt() != 0x0db0 ||
            input.match.value("productId").toInt() != 0x0076 ||
            input.match.value("boardModel").toString().trimmed().isEmpty())
            return reject("MSI Gen1 requires an explicit board model and controller identity");
        QSet<int> zones;
        QSet<QString> ids;
        for (const auto& channel : input.channels) {
            const auto value = channel.address.value("zone");
            const int zone = value.toInt(-1);
            if (!value.isDouble() || value.toDouble() != zone || zone < 0 || zone >= 18 ||
                zones.contains(zone) || channel.id.isEmpty() || ids.contains(channel.id) ||
                channel.maxLeds < 1 || channel.maxLeds > 120 || channel.fixedLeds < 0 ||
                channel.fixedLeds > channel.maxLeds || (!channel.direct && channel.nativeModes.isEmpty()))
                return reject("Invalid or duplicate MSI channel/zone");
            zones.insert(zone);
            ids.insert(channel.id);
            for (const auto& mode : channel.nativeModes)
                if (!modes.contains(mode)) return reject("Unsupported MSI native mode: " + mode);
            if (channel.direct) {
                const auto areaValue = channel.address.value("area");
                const int area = areaValue.toInt(-1);
                // Only the documented ARGB/EZ paths are enabled. The older
                // worktree found no working per-LED 0x51 path for IO COVER.
                if (!areaValue.isDouble() || areaValue.toDouble() != area ||
                    !((zone >= 0 && zone <= 2 && area == 0x0400 + zone) ||
                      (zone == 3 && area == 0x0800)))
                    return reject("Unqualified MSI direct-mode zone/area pairing");
            }
        }
        return true;
    }

    std::unique_ptr<IDriver> create(IHidDevice& transport, const DeviceProfile& profile) override
    {
        if (!acceptsProfile(profile, nullptr)) return {};
        return std::make_unique<MsiDriver>(transport, profile);
    }
};

#include "MsiModule.moc"
