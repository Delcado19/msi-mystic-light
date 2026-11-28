#include "modules/ModuleRegistry.h"
#include "output/HardwareOutput.h"
#include <QtTest>
#include "../hid/FakeHidDevice.h"
#include "components/ComponentCatalog.h"
#include "protocol/AjazzAk35iV4Protocol.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QSet>

namespace {
// AJAZZ AK35I V4 owner captures (docs/hardware-inventory/ajazz-ak35i-v4-rgb-fixtures.json).
QJsonObject ajazzTransaction(const QString& name)
{
    QFile file(AJAZZ_FIXTURE_FILE); if (!file.open(QIODevice::ReadOnly)) return {};
    for (const auto& value : QJsonDocument::fromJson(file.readAll()).object().value("transactions").toArray())
        if (value.toObject().value("name").toString() == name) return value.toObject();
    return {};
}
std::vector<std::uint8_t> ajazzNative(const QJsonObject& transaction, const QString& operation, int index)
{
    int seen = 0;
    for (const auto& value : transaction.value("operations").toArray()) {
        const auto op = value.toObject(); if (op.value("operation").toString() != operation) continue;
        if (seen++ != index) continue;
        const auto payload = QByteArray::fromHex(op.value("payload_hex").toString().toLatin1());
        std::vector<std::uint8_t> report{0}; report.insert(report.end(), payload.cbegin(), payload.cend()); return report;
    }
    return {};
}
DeviceProfile ajazzProfile() { return DeviceProfile::load(QString(PROFILE_DIRECTORY) + "/experimental/ajazz-ak35i-v4.json"); }
QVector<RgbColor> ajazzFrame(const QJsonObject& transaction)
{
    QVector<RgbColor> colors(AJAZZ_V4_LEDS);
    const auto lit = transaction.value("input_nonzero_leds").toObject();
    for (int order = 0; order < AJAZZ_V4_LEDS; ++order) {
        const auto rgb = lit.value(QString::number(AJAZZ_V4_LIGHT_IDS[order])).toArray();
        if (!rgb.isEmpty()) colors[order] = {quint8(rgb[0].toInt()), quint8(rgb[1].toInt()), quint8(rgb[2].toInt())};
    }
    return colors;
}
OutputProof ajazzProof()
{
    OutputProof proof; proof.profile = ajazzProfile(); proof.identity.path = "fake-endpoint";
    proof.identity.vendorId = 0x0c45; proof.identity.productId = 0x8009; proof.identity.interfaceNumber = 3;
    proof.identity.usagePage = 0xff13; proof.identity.usage = 1;
    proof.initial.endpoint = proof.identity.path; proof.initial.profileId = proof.profile.id;
    ChannelOutput channel; channel.channelId = "AjazzAk35iV4"; channel.software = true;
    channel.colors = ajazzFrame(ajazzTransaction("direct-frame")); proof.initial.channels = {channel};
    return proof;
}
std::vector<std::uint8_t> tagged(char tag, std::vector<std::uint8_t> bytes) { bytes.insert(bytes.begin(), std::uint8_t(tag)); return bytes; }
}

class PeripheralTest : public QObject {
    Q_OBJECT
private slots:
    void experimentalProfileInactiveByDefault();
    void ajazzProfileAccepted() {
        ModuleRegistry modules; QString error; QVERIFY2(modules.load(PERIPHERAL_MODULE_PATH,&error),qPrintable(error));
        const auto profile=ajazzProfile();
        QVERIFY2(modules.acceptsProfile(profile,&error),qPrintable(error));
        const auto rejected=[&](const std::function<void(DeviceProfile&)>& change) { auto copy=profile; change(copy); return !modules.acceptsProfile(copy,&error); };
        QVERIFY(rejected([](DeviceProfile& p){p.match["usagePage"]=0xff68;}));   // TFT collection
        QVERIFY(rejected([](DeviceProfile& p){p.match["productId"]=0xfefe;}));   // 2.4 GHz receiver
        QVERIFY(rejected([](DeviceProfile& p){p.match["usage"]=0x61;}));
        QVERIFY(rejected([](DeviceProfile& p){p.channels[0].fixedLeds=104;}));
        QVERIFY(rejected([](DeviceProfile& p){p.channels[0].id="Other";}));
        QVERIFY(rejected([](DeviceProfile& p){p.channels[0].nativeModes={"static"};}));
        // Component: 105 distinct positions, addresses in XML order.
        const auto component=loadComponentDefinition(QString(COMPONENT_DIRECTORY)+"/peripheral-ajazz-ak35i-v4.json");
        QCOMPARE(component.positions.size(),105);
        {QSet<QPair<double,double>> unique; for(const auto& p:component.positions) unique.insert({p.x(),p.y()}); QCOMPARE(unique.size(),105);}
        for(int i=0;i<105;++i) QCOMPARE(component.mapping[i],i);
    }
    void ajazzFrameSequence() {
        ModuleRegistry modules; QString error; QVERIFY2(modules.load(PERIPHERAL_MODULE_PATH,&error),qPrintable(error));
        const auto proof=ajazzProof(); const auto frame=ajazzTransaction("direct-frame");
        FakeHidDevice device; QVERIFY(device.open(0,0));
        auto driver=modules.create(proof.identity,proof.profile,device,&error); QVERIFY2(driver,qPrintable(error));
        QVERIFY(device.operations.empty()); // constructionSendsNothing
        const auto start=ajazzTransaction("global-blue-brightness2");
        device.featureReportQueue={ajazzNative(start,"get",0),ajazzNative(frame,"get",0),ajazzNative(frame,"get",1)};
        const auto colors=proof.initial.channels[0].colors;
        QVERIFY(driver->driver().pushColors("AjazzAk35iV4",colors));
        std::vector<std::vector<std::uint8_t>> expected{tagged('S',ajazzNative(start,"set",0)),{'G'}};
        expected.push_back(tagged('S',ajazzNative(frame,"set",0))); expected.push_back({'G'});
        for(int chunk=1;chunk<=8;++chunk) expected.push_back(tagged('S',ajazzNative(frame,"set",chunk)));
        expected.push_back(tagged('S',ajazzNative(frame,"set",9))); expected.push_back({'G'});
        QCOMPARE(device.operations.size(),expected.size());
        for(std::size_t i=0;i<expected.size();++i) QVERIFY2(device.operations[i]==expected[i],qPrintable(QString("operation %1").arg(i)));
        // Later frames are not re-armed with START.
        device.operations.clear(); device.featureReportQueue={ajazzNative(frame,"get",0),ajazzNative(frame,"get",1)};
        const auto next=ajazzTransaction("direct-frame-next");
        QVERIFY(driver->driver().pushColors("AjazzAk35iV4",ajazzFrame(next)));
        QCOMPARE(device.operations.size(),std::size_t(12)); // 04 20, ACK, 8 data, 04 02, ACK
        QCOMPARE(device.operations[0],tagged('S',ajazzNative(next,"set",0)));
        QCOMPARE(device.operations[8],tagged('S',ajazzNative(next,"set",7)));
    }
    void shortAckStopsTransaction() {
        ModuleRegistry modules; QString error; QVERIFY2(modules.load(PERIPHERAL_MODULE_PATH,&error),qPrintable(error));
        const auto proof=ajazzProof(); const auto frame=ajazzTransaction("direct-frame");
        const auto start=ajazzNative(ajazzTransaction("global-blue-brightness2"),"get",0);
        auto badStatus=ajazzNative(frame,"get",0); badStatus[4]=0x02; // neither done, pending nor busy
        auto wrongOpcode=ajazzNative(frame,"get",1); // 04 02 reply where 04 20 is expected
        for(const auto& reply:{std::vector<std::uint8_t>(10,0),badStatus,wrongOpcode,std::vector<std::uint8_t>{}}) {
            FakeHidDevice device; QVERIFY(device.open(0,0));
            auto driver=modules.create(proof.identity,proof.profile,device,&error); QVERIFY(driver);
            device.featureReportQueue={start,reply};
            QVERIFY(!driver->driver().pushColors("AjazzAk35iV4",proof.initial.channels[0].colors));
            QCOMPARE(device.operations.size(),std::size_t(4)); // START, ACK, 04 20, failed ACK: no data chunk follows
        }
        FakeHidDevice device; QVERIFY(device.open(0,0));
        auto driver=modules.create(proof.identity,proof.profile,device,&error); QVERIFY(driver);
        device.featureReportQueue={std::vector<std::uint8_t>(65,0)};
        QVERIFY(!driver->driver().pushColors("AjazzAk35iV4",proof.initial.channels[0].colors));
        QCOMPARE(device.operations.size(),std::size_t(2)); // failed START ACK: nothing else
    }
    void busyKeyboardSkipsFrames() {
        // Owner hardware test 2026-11-26: status 00/FF phases of ~2 s; they must not fault the output.
        ModuleRegistry modules; QString error; QVERIFY2(modules.load(PERIPHERAL_MODULE_PATH,&error),qPrintable(error));
        const auto proof=ajazzProof(); const auto frame=ajazzTransaction("direct-frame");
        const auto start=ajazzNative(ajazzTransaction("global-blue-brightness2"),"get",0);
        auto busy=ajazzNative(frame,"get",0); busy[4]=0xFF;
        auto pending=ajazzNative(frame,"get",0); pending[4]=0x00;
        FakeHidDevice device; QVERIFY(device.open(0,0));
        auto driver=modules.create(proof.identity,proof.profile,device,&error); QVERIFY(driver);
        const auto colors=proof.initial.channels[0].colors;
        // FF: frame skipped after the header, no data chunk is sent.
        device.featureReportQueue={start,busy};
        QVERIFY(driver->driver().pushColors("AjazzAk35iV4",colors));
        QCOMPARE(device.operations.size(),std::size_t(4)); // START, ACK, 04 20, busy ACK
        // 00 is re-read (no second SET) and a later 01 completes the frame normally.
        device.operations.clear(); device.featureReportQueue={pending,pending,ajazzNative(frame,"get",0),ajazzNative(frame,"get",1)};
        QVERIFY(driver->driver().pushColors("AjazzAk35iV4",colors));
        QCOMPARE(device.operations.size(),std::size_t(14)); // 04 20, G, G, G, 8 data, 04 02, G
        QCOMPARE(device.operations[1],std::vector<std::uint8_t>{'G'}); QCOMPARE(device.operations[4][0],std::uint8_t('S'));
        // A keyboard that stays busy fails visibly after 150 skipped frames in a row.
        for(int frameIndex=1;frameIndex<=151;++frameIndex) {
            device.featureReportQueue={busy};
            const bool ok=driver->driver().pushColors("AjazzAk35iV4",colors);
            QCOMPARE(ok,frameIndex<=150);
        }
    }
    void previewAcceptsAjazzProfile() {
        // The output review gate serializes a frame without hardware; ACK reads must not break it.
        ModuleRegistry modules; QString error; QVERIFY2(modules.load(PERIPHERAL_MODULE_PATH,&error),qPrintable(error));
        const auto proof=ajazzProof(); const auto frame=ajazzTransaction("direct-frame");
        const auto reports=previewOutputReports(modules,proof);
        QCOMPARE(reports.size(),11); // START, 04 20, 8 data, 04 02
        const auto native=[](const std::vector<std::uint8_t>& bytes){return QByteArray(reinterpret_cast<const char*>(bytes.data()),qsizetype(bytes.size()));};
        QCOMPARE(reports[0],native(ajazzNative(ajazzTransaction("global-blue-brightness2"),"set",0)));
        for(int i=1;i<11;++i) QCOMPARE(reports[i],native(ajazzNative(frame,"set",i-1)));
        QCOMPARE(previewOutputReports(modules,proof),reports); // stable for the connect-time recheck
    }
    void invalidFrameSendsNothing() {
        ModuleRegistry modules; QString error; QVERIFY2(modules.load(PERIPHERAL_MODULE_PATH,&error),qPrintable(error));
        const auto proof=ajazzProof();
        FakeHidDevice device; QVERIFY(device.open(0,0));
        auto driver=modules.create(proof.identity,proof.profile,device,&error); QVERIFY(driver);
        QVERIFY(!driver->driver().pushColors("AjazzAk35iV4",QVector<RgbColor>(104)));
        QVERIFY(!driver->driver().pushColors("Other",proof.initial.channels[0].colors));
        device.close(); QVERIFY(!driver->driver().pushColors("AjazzAk35iV4",proof.initial.channels[0].colors));
        QVERIFY(device.operations.empty());
    }
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
            QCOMPARE(reports.size(),model.startsWith("vulcan") ? 8 : model.startsWith("basilisk") ? 3 : 2);
            if(model.startsWith("vulcan")) { QCOMPARE(reports[0],QByteArray::fromHex("150001")); QCOMPARE(reports[1].size(),65); }
            if(model.startsWith("apex")) {
                QCOMPARE(reports[0].size(),33); QCOMPARE(quint8(reports[0][3]),100); QCOMPARE(reports[1].mid(3,3),QByteArray::fromHex("0c2238"));
                // Brightness only once per session; later frames send colors only (audit 2026-11-14).
                FakeHidDevice device; QVERIFY(device.open(0,0)); // proof.profile: the loop's profile.match[...] reads above inserted null keys into profile.
                auto driver=modules.create(proof.identity,proof.profile,device,&error); QVERIFY2(driver,qPrintable(error));
                QVERIFY(driver->driver().pushColors(channel.channelId,channel.colors)); QVERIFY(driver->driver().pushColors(channel.channelId,channel.colors));
                QCOMPARE(device.sentReports.size(),std::size_t(3)); QCOMPARE(int(device.sentReports[2][1]),0x0B);
            }
            if(model.startsWith("basilisk")) {
                // Brightness 255 first (Synapse "brightness off" leaves the device at 0), then colors + custom mode.
                QCOMPARE(reports[0].size(),91); QCOMPARE(reports[0].mid(6,6),QByteArray::fromHex("030f040000ff"));
                QCOMPARE(quint8(reports[0][89]),quint8(0x03^0x0f^0x04^0xff));
                QCOMPARE(reports[1].mid(14,3),QByteArray::fromHex("0c2238"));
                // Later frames repeat brightness only after the refresh interval.
                FakeHidDevice device; QVERIFY(device.open(0,0));
                auto driver=modules.create(proof.identity,proof.profile,device,&error); QVERIFY2(driver,qPrintable(error));
                QVERIFY(driver->driver().pushColors(channel.channelId,channel.colors)); QVERIFY(driver->driver().pushColors(channel.channelId,channel.colors));
                QCOMPARE(device.sentReports.size(),std::size_t(5));
                QTest::qSleep(1100);
                QVERIFY(driver->driver().pushColors(channel.channelId,channel.colors));
                QCOMPARE(device.sentReports.size(),std::size_t(8)); QCOMPARE(int(device.sentReports[5][8]),0x04);
            }
            if(model.startsWith("kone")) { QCOMPARE(reports[0].size(),6); QCOMPARE(reports[1].size(),46); }
            profile.channels[0].fixedLeds=1; QVERIFY(!modules.acceptsProfile(profile,&error));
            proof.initial.channels[0].colors.clear(); QVERIFY_EXCEPTION_THROWN(previewOutputReports(modules,proof),std::runtime_error);
        }
    }
};
void PeripheralTest::experimentalProfileInactiveByDefault()
{
    // Decision D1 (2026-11-26): the V4 profile shares VID:PID with AK820 MAX PLUS / V3 MAX and loads only on opt-in.
    const auto names = [](const QFileInfoList& files) { QStringList result; for (const auto& file : files) result.append(file.fileName()); return result; };
    const auto off = names(deviceProfileFiles(QString(PROFILE_DIRECTORY), false));
    QVERIFY(!off.isEmpty()); QVERIFY(!off.contains("ajazz-ak35i-v4.json"));
    const auto on = names(deviceProfileFiles(QString(PROFILE_DIRECTORY), true));
    QVERIFY(on.contains("ajazz-ak35i-v4.json")); QCOMPARE(on.size(), off.size() + 1);
}

QTEST_GUILESS_MAIN(PeripheralTest)
#include "test_peripherals.moc"
