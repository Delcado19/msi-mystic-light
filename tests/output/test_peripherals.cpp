#include "modules/ModuleRegistry.h"
#include "output/HardwareOutput.h"
#include <QtTest>

class PeripheralTest : public QObject {
    Q_OBJECT
private slots:
    void pairing() {
        HidDescriptor led; led.path="led"; led.vendorId=0x1e7d; led.productId=0x307a; led.interfaceNumber=3; led.usagePage=1;
        auto control=led; control.path="control"; control.interfaceNumber=1; control.usagePage=11;
        const auto identity=deviceIdentity(led,{});
        QCOMPARE(vulcanControlIdentity(identity,{led,control}).path,QString("control"));
        QVERIFY_EXCEPTION_THROWN(vulcanControlIdentity(identity,{led}),std::runtime_error);
        auto second=led; second.path="other-led";
        QVERIFY_EXCEPTION_THROWN(vulcanControlIdentity(identity,{led,second,control}),std::runtime_error);
        control.productId=0x3098;
        QVERIFY_EXCEPTION_THROWN(vulcanControlIdentity(identity,{led,control}),std::runtime_error);
    }
    void reports() {
        ModuleRegistry modules; QString error; QVERIFY2(modules.load(PERIPHERAL_MODULE_PATH,&error),qPrintable(error));
        for(const auto& model:QStringList{"kone-2e27","basilisk-0099","apex3-161a","vulcan-307a"}) {
            auto profile=DeviceProfile::load(QString(PROFILE_DIRECTORY)+"/"+model+".json");
            QVERIFY(modules.acceptsProfile(profile,&error));
            OutputProof proof; proof.profile=profile; proof.identity.path="fake-endpoint";
            proof.identity.vendorId=profile.match["vendorId"].toInt(); proof.identity.productId=profile.match["productId"].toInt();
            proof.identity.interfaceNumber=profile.match["interfaceNumber"].toInt(); proof.identity.usagePage=profile.match["usagePage"].toInt(-1);
            proof.initial.endpoint=proof.identity.path; proof.initial.profileId=profile.id;
            ChannelOutput channel; channel.channelId=profile.channels[0].id; channel.software=true;
            channel.colors.fill({12,34,56},profile.channels[0].fixedLeds); proof.initial.channels={channel};
            const auto reports=previewOutputReports(modules,proof);
            QCOMPARE(reports.size(),model.startsWith("vulcan") ? 8 : 2);
            if(model.startsWith("vulcan")) { QCOMPARE(reports[0],QByteArray::fromHex("150001")); QCOMPARE(reports[1].size(),65); }
            if(model.startsWith("apex")) { QCOMPARE(reports[0].size(),33); QCOMPARE(quint8(reports[0][3]),100); QCOMPARE(reports[1].mid(3,3),QByteArray::fromHex("0c2238")); }
            if(model.startsWith("basilisk")) { QCOMPARE(reports[0].size(),91); QCOMPARE(reports[0].mid(14,3),QByteArray::fromHex("0c2238")); }
            if(model.startsWith("kone")) { QCOMPARE(reports[0].size(),6); QCOMPARE(reports[1].size(),46); }
            profile.channels[0].fixedLeds=1; QVERIFY(!modules.acceptsProfile(profile,&error));
            proof.initial.channels[0].colors.clear(); QVERIFY_EXCEPTION_THROWN(previewOutputReports(modules,proof),std::runtime_error);
        }
    }
};
QTEST_GUILESS_MAIN(PeripheralTest)
#include "test_peripherals.moc"
