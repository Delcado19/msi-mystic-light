// SPDX-License-Identifier: GPL-2.0-or-later
// OpenRGB HPOmen30LController, aac7e5511439e020ddae88e78bd400c71b8d4a9c.
// Source comparison, supported scope and unverified hardware status: docs/HP_OMEN_MIGRATION.md.
#include "modules/IDriverModule.h"
#include <QObject>
#include <QSet>
#include <array>

namespace {
const QMap<QString, int> modes{{"static",1},{"off",5},{"breathing",6},
    {"color-cycle",7},{"blinking",8},{"wave",9},{"radial",10}};

class HpOmenDriver final : public IDriver {
public:
    HpOmenDriver(IHidDevice& transport,const DeviceProfile& profile) : transport_(transport)
    {
        for(const auto& channel:profile.channels)channels_.insert(channel.id,channel);
    }

    bool pushColors(const QString& id,const QVector<RgbColor>& colors) override
    {
        const auto channel=channels_.constFind(id);
        if(channel==channels_.cend() || !channel->direct || colors.size()!=1)return false;
        return send(channel->address.value("zone").toInt(),4,colors,5,0);
    }

    bool setNativeMode(const QString& id,const NativeEffect& effect) override
    {
        const auto channel=channels_.constFind(id);
        if(channel==channels_.cend() || !channel->nativeModes.contains(effect.id) ||
            !modes.contains(effect.id) || effect.inward || effect.brightness<0 || effect.brightness>5 ||
            effect.speed<0 || effect.speed>2)return false;
        const int mode=modes.value(effect.id);
        if((mode==1 && effect.colors.size()!=1) ||
            (mode>=6 && (effect.colors.isEmpty() || effect.colors.size()>6)))return false;
        return send(channel->address.value("zone").toInt(),mode,effect.colors,effect.brightness,effect.speed);
    }

private:
    bool send(int zone,int mode,const QVector<RgbColor>& colors,int brightness,int speed)
    {
        if(!transport_.isOpen())return false;
        std::array<std::uint8_t,58> packet{};
        packet[2]=0x12;packet[3]=mode;packet[0x36]=zone;
        // Only powered-on settings: never change the independent suspend profile.
        packet[0x37]=1;
        if(mode==5)return transport_.writeOutputReport(packet.data(),packet.size());
        packet[0x30]=brightness*20;
        packet[0x31]=mode==4 ? 4 : mode==1 ? 2 : 10;
        packet[4]=static_cast<std::uint8_t>(colors.size());
        if(mode>=6)packet[0x39]=speed+1;
        for(int index=0;index<colors.size();++index) {
            packet[5]=index+1;
            const int offset=(mode==4 ? 9 : 8)+(zone-1)*(mode==4 ? 4 : 3);
            if(mode==4)packet[offset-1]=100;
            packet[offset]=colors[index].r;packet[offset+1]=colors[index].g;packet[offset+2]=colors[index].b;
            // Stop on partial palette failure; the caller must see that the update failed.
            if(!transport_.writeOutputReport(packet.data(),packet.size()))return false;
        }
        return true;
    }
    IHidDevice& transport_;
    QMap<QString,ChannelProfile> channels_;
};
}

class HpOmenModule final : public QObject,public IDriverModule {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DRIVER_MODULE_IID FILE "module.json")
    Q_INTERFACES(IDriverModule)
public:
    bool acceptsProfile(const DeviceProfile& profile,QString* error) const override
    {
        const auto reject=[error](const QString& text){if(error)*error=text;return false;};
        if(error)error->clear();
        if(profile.driverId!="hp.omen30l" || profile.channels.isEmpty() ||
            profile.match.value("vendorId").toInt()!=0x103c ||
            profile.match.value("productId").toInt()!=0x84fd ||
            profile.match.value("transport").toString("hid")!="hid")
            return reject("HP Omen 30L requires HID identity 103C:84FD");
        QSet<int> zones;QSet<QString> ids;
        for(const auto& channel:profile.channels) {
            const auto value=channel.address.value("zone");const int zone=value.toInt(-1);
            if(!value.isDouble() || value.toDouble()!=zone || zone<1 || zone>7 || zones.contains(zone) ||
                channel.id.isEmpty() || ids.contains(channel.id) || channel.maxLeds!=1 || channel.fixedLeds!=1 ||
                (!channel.direct && channel.nativeModes.isEmpty()))
                return reject("HP Omen requires unique zones 1..7 with one shared color per zone");
            for(const auto& mode:channel.nativeModes)
                if(!modes.contains(mode))return reject("Unsupported HP Omen native mode: "+mode);
            zones.insert(zone);ids.insert(channel.id);
        }
        return true;
    }
    std::unique_ptr<IDriver> create(IHidDevice& transport,const DeviceProfile& profile) override
    {
        if(!acceptsProfile(profile,nullptr))return {};
        return std::make_unique<HpOmenDriver>(transport,profile);
    }
};
#include "HpOmenModule.moc"
