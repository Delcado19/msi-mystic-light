// Restore the original Lighting Studio peripheral encoders behind the shared output review gate.
#include "modules/IDriverModule.h"
#include "protocol/RoccatKoneProtocol.h"
#include "protocol/RazerBasiliskProtocol.h"
#include "protocol/SteelSeriesApex3Protocol.h"
#include "protocol/RoccatVulcanProtocol.h"
#include "protocol/AjazzAk35iV4Protocol.h"
#include <QObject>
#include <QJsonArray>
#include <chrono>
#include <thread>

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
        if(protocol=="basilisk") {
            // Synapse "brightness off" stores 0 on the device and then sends nothing, so it can keep running
            // (macros, key remaps) without fighting our frames; we restore 255 once a second in case Synapse
            // sets 0 again on start, profile switch or resume (unmeasured). USB capture 2026-11-26.
            const auto now=std::chrono::steady_clock::now();
            if(!initialized_ || now-brightnessSent_>=std::chrono::seconds(1)) {
                if(!feature(BuildBasiliskBrightnessReport(0xFF))) return false;
                initialized_=true; brightnessSent_=now;
            }
            return feature(BuildBasiliskColorsReport(rgb)) && feature(BuildBasiliskModeCustomReport());
        }
        if(protocol=="apex3") {
            // Brightness is device state; sending it with every frame doubled the writes (audit 2026-11-14).
            if(!initialized_ && !output(BuildApex3BrightnessPacket(100))) return false;
            initialized_=true; return output(BuildApex3ColorsPacket(rgb));
        }
        if(protocol=="ajazz-ak35i-v4") {
            // Captured Direct RGB transaction, docs/AJAZZ_AK35I_V4_PROTOCOL.md section 4: START once per
            // session, then 04 20 + ACK, eight data reports, 04 02 + ACK. Nothing is replayed and no
            // persistent command is sent.
            // Owner hardware test 2026-11-26 (firmware 0109): the keyboard regularly answers an ACK with
            // status 00 or FF for up to ~2 s, then recovers by itself. Failing the frame faulted the output
            // and the reconnect left the keys black for 3 s. A busy answer now skips only this frame; the
            // next frame starts a fresh transaction. More than maxBusyFrames in a row still fail visibly.
            const auto chunks=BuildAjazzDirectChunks(rgb);
            const auto busy=[this] { return ++busyFrames_<=maxBusyFrames; };
            if(!initialized_) {
                const auto start=ajazzCommand(0x18);
                if(start==Ack::Failed) return false;
                if(start==Ack::Busy) return busy();
                initialized_=true;
            }
            auto ack=ajazzCommand(0x20,8);
            if(ack==Ack::Ok) {
                for(const auto& chunk:chunks) if(!feature(chunk)) return false;
                ack=ajazzCommand(0x02);
            }
            if(ack==Ack::Failed) return false;
            if(ack==Ack::Busy) return busy();
            busyFrames_=0; return true;
        }
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
    // Feature SET, then a 65-byte GET (byte 0 = report ID 0, verified on Windows hidapi 2026-11-26) that
    // must echo the opcode with status 01. A well-formed echo with status 00 (not done yet) is re-read a
    // few times without repeating the SET; FF, or 00 that never clears, means busy. Anything else fails.
    enum class Ack { Ok, Busy, Failed };
    Ack ajazzCommand(std::uint8_t opcode,std::uint8_t arg8=0) {
        if(!feature(BuildAjazzCommand(opcode,arg8))) return Ack::Failed;
        for(int read=0;read<4;++read) {
            AjazzReport reply{};
            const auto length=transport_.getFeatureReport(reply.data(),reply.size());
            if(AjazzAckOk(reply.data(),length,opcode)) return Ack::Ok;
            const bool echo=length==AJAZZ_V4_REPORT && reply[0]==0 && reply[1]==0x04 && reply[2]==opcode;
            if(!echo || (reply[4]!=0x00 && reply[4]!=0xFF)) return Ack::Failed;
            if(reply[4]==0xFF) return Ack::Busy;
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
        return Ack::Busy;
    }
    static constexpr int maxBusyFrames=150; // about 5 s at 30 fps; busy phases seen on hardware lasted ~2 s
    IHidDevice& transport_; ChannelProfile channel_; bool initialized_=false; int busyFrames_=0;
    std::chrono::steady_clock::time_point brightnessSent_;
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
        // AK35I V4: only the FF13:0001 control collection; never the FF68 TFT collection or the FEFE receiver.
        const bool ajazz=protocol=="ajazz-ak35i-v4" && vid==0x0c45 && pid==0x8009 && iface==3 && usage==0xff13 &&
            profile.match.value("usage").toInt(-1)==1 && channel.id=="AjazzAk35iV4";
        const int count=ajazz ? AJAZZ_V4_LEDS : vulcan ? 144 : apex ? 10 : 11;
        if(!(kone || basilisk || apex || vulcan || ajazz) || !channel.direct || !channel.nativeModes.isEmpty() || channel.maxLeds!=count || channel.fixedLeds!=count) return reject();
        return true;
    }
    std::unique_ptr<IDriver> create(IHidDevice& transport,const DeviceProfile& profile) override {
        if(!acceptsProfile(profile,nullptr)) return {};
        return std::make_unique<PeripheralDriver>(transport,profile.channels[0]);
    }
};
#include "PeripheralModule.moc"
