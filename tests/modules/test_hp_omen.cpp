#include "modules/ModuleRegistry.h"
#include "hid/FakeHidDevice.h"
#include <QtTest>

namespace {
DeviceProfile profile(){return DeviceProfile::load(TEST_PROFILE_PATH);}
DeviceIdentity identity(){DeviceIdentity result;result.path="fake-omen";result.vendorId=0x103c;result.productId=0x84fd;return result;}
class OutputOnlyHid final : public FakeHidDevice {
public:
    bool sendFeatureReport(const std::uint8_t*,std::size_t) override {++featureWrites;return false;}
    int featureWrites=0;
};
}

class HpOmenTest : public QObject {
    Q_OBJECT
private slots:
    void profileValidation()
    {
        ModuleRegistry modules;QString error;QVERIFY2(modules.load(TEST_MODULE_PATH,&error),qPrintable(error));
        const auto good=profile();QVERIFY(modules.acceptsProfile(good,&error));
        auto bad=good;bad.match["productId"]=1;QVERIFY(!modules.acceptsProfile(bad,&error));
        bad=good;bad.match["transport"]="pawnio";QVERIFY(!modules.acceptsProfile(bad,&error));
        for(double value:{0.,8.,1.5}) {bad=good;bad.channels[0].address["zone"]=value;QVERIFY(!modules.acceptsProfile(bad,&error));}
        bad=good;bad.channels[1].address["zone"]=1;QVERIFY(!modules.acceptsProfile(bad,&error));
        bad=good;bad.channels[1].id=bad.channels[0].id;QVERIFY(!modules.acceptsProfile(bad,&error));
        bad=good;bad.channels[0].maxLeds=2;QVERIFY(!modules.acceptsProfile(bad,&error));
        bad=good;bad.channels[0].fixedLeds=0;QVERIFY(!modules.acceptsProfile(bad,&error));
        bad=good;bad.channels[0].nativeModes={"unknown"};QVERIFY(!modules.acceptsProfile(bad,&error));
        bad=good;bad.channels.clear();QVERIFY(!modules.acceptsProfile(bad,&error));
    }

    void allZoneDirectPackets()
    {
        ModuleRegistry modules;QString error;QVERIFY(modules.load(TEST_MODULE_PATH,&error));
        OutputOnlyHid hid;QVERIFY(hid.open(0x103c,0x84fd));auto data=profile();
        auto driver=modules.create(identity(),data,hid,&error);QVERIFY2(driver,qPrintable(error));
        QVERIFY(hid.sentReports.empty());QCOMPARE(hid.getFeatureReportCalls,0);QCOMPARE(hid.readInputReportCalls,0);
        for(const auto& channel:data.channels) {
            const int zone=channel.address.value("zone").toInt();
            QVERIFY(driver->driver().pushColors(channel.id,{{0x12,0x34,0x56}}));
            // Independent golden layout from upstream: 58-byte output, zone-indexed ARGB slot, powered-on bank.
            std::vector<std::uint8_t> expected(58,0);expected[2]=0x12;expected[3]=4;expected[4]=1;expected[5]=1;
            expected[0x30]=100;expected[0x31]=4;expected[0x36]=zone;expected[0x37]=1;
            const int offset=8+(zone-1)*4;expected[offset]=100;expected[offset+1]=0x12;expected[offset+2]=0x34;expected[offset+3]=0x56;
            QVERIFY(hid.sentReports.back()==expected);
        }
        QCOMPARE(hid.sentReports.size(),std::size_t{7});QCOMPARE(hid.featureWrites,0);
    }

    void nativePacketsAndFailure()
    {
        ModuleRegistry modules;QString error;QVERIFY(modules.load(TEST_MODULE_PATH,&error));
        OutputOnlyHid hid;QVERIFY(hid.open(0x103c,0x84fd));auto driver=modules.create(identity(),profile(),hid,&error);QVERIFY(driver);
        NativeEffect effect;effect.id="static";effect.colors={{10,20,30}};effect.brightness=3;
        QVERIFY(driver->driver().setNativeMode("TopFan",effect));
        std::vector<std::uint8_t> expected(58,0);expected[2]=0x12;expected[3]=1;expected[4]=1;expected[5]=1;
        expected[26]=10;expected[27]=20;expected[28]=30;expected[0x30]=60;expected[0x31]=2;expected[0x36]=7;expected[0x37]=1;
        QVERIFY(hid.sentReports.back()==expected);
        const QStringList modes{"breathing","color-cycle","blinking","wave","radial"};
        for(int mode=0;mode<modes.size();++mode) {
            effect.id=modes[mode];effect.speed=2;effect.colors={{1,2,3},{4,5,6}};
            const auto before=hid.sentReports.size();QVERIFY(driver->driver().setNativeMode("Logo",effect));
            QCOMPARE(hid.sentReports.size(),before+2);
            for(int index=0;index<2;++index) {
                std::vector<std::uint8_t> packet(58,0);packet[2]=0x12;packet[3]=6+mode;packet[4]=2;packet[5]=index+1;
                packet[8]=1+index*3;packet[9]=2+index*3;packet[10]=3+index*3;
                packet[0x30]=60;packet[0x31]=10;packet[0x36]=1;packet[0x37]=1;packet[0x39]=3;
                QVERIFY(hid.sentReports[before+index]==packet);
            }
        }
        effect.id="off";effect.colors.clear();QVERIFY(driver->driver().setNativeMode("Logo",effect));
        expected.assign(58,0);expected[2]=0x12;expected[3]=5;expected[0x36]=1;expected[0x37]=1;
        QVERIFY(hid.sentReports.back()==expected);
        hid.sendSucceeds=false;QVERIFY(!driver->driver().pushColors("Logo",{{1,2,3}}));
        effect.id="breathing";effect.colors={{1,2,3},{4,5,6}};const auto before=hid.sentReports.size();
        QVERIFY(!driver->driver().setNativeMode("Logo",effect));QCOMPARE(hid.sentReports.size(),before+1);
        hid.close();QVERIFY(!driver->driver().pushColors("Logo",{{1,2,3}}));QCOMPARE(hid.sentReports.size(),before+1);
    }

    void invalidEffectsNeverWrite()
    {
        ModuleRegistry modules;QString error;QVERIFY(modules.load(TEST_MODULE_PATH,&error));
        OutputOnlyHid hid;QVERIFY(hid.open(0x103c,0x84fd));auto driver=modules.create(identity(),profile(),hid,&error);QVERIFY(driver);
        QVERIFY(!driver->driver().pushColors("unknown",{{1,2,3}}));QVERIFY(!driver->driver().pushColors("Logo",{}));
        QVERIFY(!driver->driver().pushColors("Logo",{{1,2,3},{4,5,6}}));
        NativeEffect effect;effect.id="unknown";QVERIFY(!driver->driver().setNativeMode("Logo",effect));
        effect.id="static";effect.colors.clear();QVERIFY(!driver->driver().setNativeMode("Logo",effect));
        effect.id="breathing";effect.colors=QVector<RgbColor>(7);QVERIFY(!driver->driver().setNativeMode("Logo",effect));
        effect.colors={{1,2,3}};effect.speed=3;QVERIFY(!driver->driver().setNativeMode("Logo",effect));
        effect.speed=0;effect.brightness=6;QVERIFY(!driver->driver().setNativeMode("Logo",effect));
        effect.brightness=5;effect.inward=true;QVERIFY(!driver->driver().setNativeMode("Logo",effect));
        QVERIFY(hid.sentReports.empty());
        auto wrong=identity();wrong.productId=1;QVERIFY(!modules.create(wrong,profile(),hid,&error));
    }
};
QTEST_GUILESS_MAIN(HpOmenTest)
#include "test_hp_omen.moc"
