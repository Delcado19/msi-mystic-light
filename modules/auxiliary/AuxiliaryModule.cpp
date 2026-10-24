// SPDX-License-Identifier: GPL-2.0-or-later
#include "modules/IDriverModule.h"
#include <QObject>
#include <QByteArray>
namespace {
class AuxiliaryDriver final : public IDriver {
public:
    AuxiliaryDriver(IHidDevice& transport,ChannelProfile channel):transport_(transport),channel_(std::move(channel)) {}
    bool setNativeMode(const QString&,const NativeEffect&) override { return false; }
    bool pushColors(const QString& channel,const QVector<RgbColor>& colors) override {
        if(channel!=channel_.id || colors.size()!=channel_.fixedLeds || !transport_.isOpen()) return false;
        QByteArray payload; payload.reserve(colors.size()*3);
        for(const auto& color:colors) { payload.append(char(color.r));payload.append(char(color.g));payload.append(char(color.b)); }
        return transport_.sendFeatureReport(reinterpret_cast<const std::uint8_t*>(payload.constData()),payload.size());
    }
private:
    IHidDevice& transport_; ChannelProfile channel_;
};
}
class AuxiliaryModule final : public QObject, public IDriverModule {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DRIVER_MODULE_IID FILE "module.json")
    Q_INTERFACES(IDriverModule)
public:
    bool acceptsProfile(const DeviceProfile& profile,QString* error) const override {
        const auto reject=[&] { if(error) *error="Unqualified auxiliary lighting profile"; return false; };
        if(profile.driverId!="lighting.auxiliary" || profile.channels.size()!=1) return reject();
        const auto& channel=profile.channels[0]; const auto transport=profile.match.value("transport").toString();
        const bool gpu=profile.id=="gigabyte-aero-5080" && transport=="nvapi" && channel.id=="GigabyteAero5080" &&
            profile.match.value("endpoint")=="nvapi://10de:2c02/1458:4179" && profile.match.value("vendorId").toInt()==0x10de && profile.match.value("productId").toInt()==0x2c02;
        bool ram=false;
        for(const auto& suffix:QStringList{"71","73"})
            ram|=profile.id=="xpg-ram-"+suffix && transport=="pawnio" && channel.id=="XpgRam"+(suffix=="71" ? QString("1") : QString("2")) &&
                profile.match.value("endpoint")=="pawnio://amd-piix4/0/0x"+suffix && profile.match.value("vendorId").toInt()==0x1022 &&
                profile.match.value("productId").toInt()==0x790b && channel.fixedLeds==8 && channel.maxLeds==8;
        if((!gpu && !ram) || !channel.direct || !channel.nativeModes.isEmpty() || (gpu && (channel.fixedLeds!=1 || channel.maxLeds!=1))) return reject();
        if(error) error->clear(); return true;
    }
    std::unique_ptr<IDriver> create(IHidDevice& transport,const DeviceProfile& profile) override {
        if(!acceptsProfile(profile,nullptr)) return {};
        return std::make_unique<AuxiliaryDriver>(transport,profile.channels[0]);
    }
};
#include "AuxiliaryModule.moc"
