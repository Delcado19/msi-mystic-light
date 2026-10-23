// Restore the original Lighting Studio peripheral encoders behind the shared output review gate.
#include "modules/IDriverModule.h"
#include "protocol/RoccatKoneProtocol.h"
#include "protocol/RazerBasiliskProtocol.h"
#include "protocol/SteelSeriesApex3Protocol.h"
#include "protocol/RoccatVulcanProtocol.h"
#include <QObject>
#include <QJsonArray>

namespace {
class PeripheralDriver final : public IDriver {
public:
    PeripheralDriver(IHidDevice& transport, ChannelProfile channel) : transport_(transport), channel_(std::move(channel)) {}
    bool setNativeMode(const QString&,const NativeEffect&) override { return false; }
    bool pushColors(const QString& channel,const QVector<RgbColor>& colors) override {
        if(!transport_.isOpen() || channel!=channel_.id || colors.size()!=channel_.fixedLeds) return false;
        std::vector<MsiColor> rgb; for(const auto color:colors) rgb.push_back({color.r,color.g,color.b});
        const auto protocol=channel_.address.value("protocol").toString();
        if(protocol=="kone") {
            if(!initialized_ && !feature(BuildKoneInitReport())) return false;
            initialized_=true; return feature(BuildKoneColorsReport(rgb));
        }
        if(protocol=="basilisk") return feature(BuildBasiliskColorsReport(rgb)) && feature(BuildBasiliskModeCustomReport());
        if(protocol=="apex3") return output(BuildApex3BrightnessPacket(100)) && output(BuildApex3ColorsPacket(rgb));
        if(protocol=="vulcan") {
            // Paired transport routes feature reports to control, output reports to the exact LED endpoint.
            if(!initialized_ && !feature(BuildVulcanEnableDirectReport(true))) return false;
            initialized_=true;
            for(const auto& packet:BuildVulcanColorPackets(rgb)) if(!output(packet)) return false;
            return true;
        }
        return false;
    }
private:
    template<class Packet> bool feature(const Packet& packet) { return transport_.sendFeatureReport(packet.data(),packet.size()); }
    template<class Packet> bool output(const Packet& packet) { return transport_.writeOutputReport(packet.data(),packet.size()); }
    IHidDevice& transport_; ChannelProfile channel_; bool initialized_=false;
};
}
class PeripheralModule final : public QObject, public IDriverModule {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DRIVER_MODULE_IID FILE "module.json")
    Q_INTERFACES(IDriverModule)
public:
    bool acceptsProfile(const DeviceProfile& profile,QString* error) const override {
        const auto reject=[&] { if(error) *error="Invalid peripheral protocol/profile selectors"; return false; };
        if(profile.driverId!="lighting.peripherals" || profile.channels.size()!=1) return reject();
        const auto& channel=profile.channels[0]; const auto protocol=channel.address.value("protocol").toString();
        const int vid=profile.match.value("vendorId").toInt(), pid=profile.match.value("productId").toInt();
        const int iface=profile.match.value("interfaceNumber").toInt(-1), usage=profile.match.value("usagePage").toInt(-1);
        const bool kone=protocol=="kone" && vid==0x1e7d && (pid==0x2e27 || pid==0x2e2c) && iface==0 && usage==11 && channel.id=="RoccatKone";
        const bool basilisk=protocol=="basilisk" && vid==0x1532 && pid==0x0099 && iface==3 && usage==12 && channel.id=="RazerBasiliskV3";
        const bool apex=protocol=="apex3" && vid==0x1038 && pid==0x161a && iface==3 && channel.id=="SteelSeriesApex3";
        const bool vulcan=protocol=="vulcan" && vid==0x1e7d && (pid==0x307a || pid==0x3098) && iface==3 && usage==1 && channel.id=="RoccatVulcan";
        const int count=vulcan ? 144 : apex ? 10 : 11;
        if(!(kone || basilisk || apex || vulcan) || !channel.direct || !channel.nativeModes.isEmpty() || channel.maxLeds!=count || channel.fixedLeds!=count) return reject();
        return true;
    }
    std::unique_ptr<IDriver> create(IHidDevice& transport,const DeviceProfile& profile) override {
        if(!acceptsProfile(profile,nullptr)) return {};
        return std::make_unique<PeripheralDriver>(transport,profile.channels[0]);
    }
};
#include "PeripheralModule.moc"
