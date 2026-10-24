#include "output/OutputSession.h"
#include "protocol/MsiProtocol.h"
#include <QtTest>
#include <stdexcept>

namespace {
struct Recording {
    int opens = 0, closes = 0, destroyed = 0, failAt = 0;
    bool opened = true, throwWrite = false;
    std::vector<std::vector<std::uint8_t>> reports;
};
class Transport : public IHidDevice {
public:
    explicit Transport(std::shared_ptr<Recording> record) : record_(std::move(record)) {}
    ~Transport() override { ++record_->destroyed; }
    bool open(std::uint16_t, std::uint16_t) override { ++record_->opens; record_->opened = true; return true; }
    void close() override { ++record_->closes; record_->opened = false; }
    bool isOpen() const override { return record_->opened; }
    bool sendFeatureReport(const std::uint8_t* bytes, std::size_t length) override {
        if (!record_->opened) return false;
        record_->reports.emplace_back(bytes,bytes+length);
        if (record_->throwWrite) throw std::runtime_error("Simulated transport exception");
        return int(record_->reports.size()) != record_->failAt;
    }
private:
    std::shared_ptr<Recording> record_;
};
DeviceIdentity identity()
{
    DeviceIdentity result; result.path="fake-hid-path"; result.vendorId=0x0db0; result.productId=0x0076;
    result.boardManufacturer="Micro-Star International Co., Ltd.";
    result.boardModel="MPG B850 EDGE TI WIFI (MS-7E62)";
    return result;
}
OutputFrame frame(const DeviceProfile& profile)
{
    OutputFrame frame; frame.endpoint=identity().path; frame.profileId=profile.id;
    ChannelOutput software; software.channelId="JARGB1"; software.software=true; software.colors={{1,2,3}};
    ChannelOutput native; native.channelId="IOShield"; native.native.id="steady"; native.native.colors={{11,22,33}};
    frame.channels={software,native}; return frame;
}
}

class OutputTest : public QObject {
    Q_OBJECT
private slots:
    void latestFrameAndNativeCache();
    void invalidFrames();
    void failureAndReconnect();
    void lifecycle();
    void documentPreparation();
    void animatedFanHeaders();
};

void OutputTest::latestFrameAndNativeCache()
{
    ModuleRegistry modules; QString error; QVERIFY2(modules.load(TEST_MODULE_PATH,&error),qPrintable(error));
    const auto profile=DeviceProfile::load(TEST_PROFILE_PATH);
    auto record=std::make_shared<Recording>(); OutputSession session;
    QVERIFY(session.connect(modules,identity(),profile,std::make_unique<Transport>(record)));
    QCOMPARE(record->opens,0); QVERIFY(record->reports.empty()); QVERIFY(!session.hasPendingFrame());
    auto first=frame(profile), latest=first; latest.channels[0].colors={{200,100,50},{9,8,7}};
    QVERIFY(session.stage(first)); QVERIFY(session.stage(latest)); QVERIFY(record->reports.empty());
    QVERIFY(session.flush()); QVERIFY(!session.hasPendingFrame()); QCOMPARE(record->reports.size(),size_t(3));
    QCOMPARE(record->reports[0][0],std::uint8_t(0x50)); QCOMPARE(record->reports[1][0],std::uint8_t(0x51));
    const auto expected=BuildReport0x51(MsiGen1AdvArea::JARGB1,{{200,100,50},{9,8,7}});
    QVERIFY(record->reports[1]==std::vector<std::uint8_t>(expected.begin(),expected.end()));
    QCOMPARE(record->reports[2][0],std::uint8_t(0x50));
    QVERIFY(session.flush()); QCOMPARE(record->reports.size(),size_t(3));
    QVERIFY(session.stage(latest)); QVERIFY(session.flush()); QCOMPARE(record->reports.size(),size_t(4));
    QCOMPARE(record->reports.back()[0],std::uint8_t(0x51)); // Unchanged native effect is not resent.
    latest.channels={latest.channels[1]};
    auto count=record->reports.size();
    for(int change=0;change<5;++change) {
        auto& effect=latest.channels[0].native;
        if(change==0) effect.colors[0].b=44;
        if(change==1) effect.brightness=3;
        if(change==2) effect.speed=2;
        if(change==3) effect.inward=true;
        if(change==4) effect.id="breathing";
        QVERIFY(session.stage(latest)); QVERIFY(session.flush()); QCOMPARE(record->reports.size(),++count);
        QVERIFY(session.stage(latest)); QVERIFY(session.flush()); QCOMPARE(record->reports.size(),count);
    }
    // Native -> software -> native on the same channel must clear the successful native cache.
    auto switchFrame=frame(profile); switchFrame.channels.resize(1);
    switchFrame.channels[0].software=false; switchFrame.channels[0].colors.clear();
    QVERIFY(session.stage(switchFrame)); QVERIFY(session.flush());
    switchFrame.channels[0].software=true; switchFrame.channels[0].colors={{4,5,6}};
    QVERIFY(session.stage(switchFrame)); QVERIFY(session.flush());
    count=record->reports.size();
    switchFrame.channels[0].software=false; switchFrame.channels[0].colors.clear();
    QVERIFY(session.stage(switchFrame)); QVERIFY(session.flush()); QCOMPARE(record->reports.size(),count+1);
    session.disconnect(); QCOMPARE(session.state(),OutputSession::State::Disconnected);
    QCOMPARE(record->closes,1); QCOMPARE(record->destroyed,1);
}

void OutputTest::invalidFrames()
{
    ModuleRegistry modules; QString error; QVERIFY(modules.load(TEST_MODULE_PATH,&error));
    const auto profile=DeviceProfile::load(TEST_PROFILE_PATH);
    auto record=std::make_shared<Recording>(); OutputSession session;
    QVERIFY(session.connect(modules,identity(),profile,std::make_unique<Transport>(record)));
    const auto valid=frame(profile);
    auto reject=[&](OutputFrame invalid) {
        QVERIFY(session.stage(valid)); QVERIFY(!session.stage(invalid)); QVERIFY(!session.hasPendingFrame());
        QVERIFY(!session.error().isEmpty()); QCOMPARE(session.state(),OutputSession::State::Connected);
        QVERIFY(session.flush()); QVERIFY(record->reports.empty());
    };
    auto invalid=valid; invalid.endpoint="another-path"; reject(invalid);
    invalid=valid; invalid.profileId="another-profile"; reject(invalid);
    invalid=valid; invalid.channels[1].channelId="unknown"; reject(invalid);
    invalid=valid; invalid.channels.append(invalid.channels[0]); reject(invalid);
    invalid=valid; invalid.channels[0].colors.clear(); reject(invalid);
    invalid=valid; invalid.channels[0].colors.resize(121); reject(invalid);
    invalid=valid; invalid.channels[0].channelId="JRGB1"; invalid.channels.resize(1); reject(invalid);
    invalid=valid; invalid.channels[1].native.id="unknown"; reject(invalid);
    invalid=valid; invalid.channels[1].native.colors.clear(); reject(invalid);
    invalid=valid; invalid.channels[1].native.brightness=6; reject(invalid);
    invalid=valid; invalid.channels[1].native.speed=-1; reject(invalid);
    invalid=valid; invalid.channels[1].colors={{1,2,3}}; reject(invalid);
    auto empty=valid; empty.channels.clear(); QVERIFY(session.stage(empty)); QVERIFY(session.flush());
    QVERIFY(record->reports.empty());
    auto fixedProfile=profile; fixedProfile.channels[0].fixedLeds=2;
    auto fixedRecord=std::make_shared<Recording>(); OutputSession fixed;
    QVERIFY(fixed.connect(modules,identity(),fixedProfile,std::make_unique<Transport>(fixedRecord)));
    QVERIFY(!fixed.stage(valid)); QVERIFY(fixedRecord->reports.empty());
    auto two=valid; two.channels[0].colors.append({4,5,6});
    QVERIFY(fixed.stage(two)); QVERIFY(fixed.flush()); QCOMPARE(fixedRecord->reports.size(),size_t(3));
}

void OutputTest::failureAndReconnect()
{
    ModuleRegistry modules; QString error; QVERIFY(modules.load(TEST_MODULE_PATH,&error));
    const auto profile=DeviceProfile::load(TEST_PROFILE_PATH);
    for(int failure=1;failure<=3;++failure) {
        auto record=std::make_shared<Recording>(); record->failAt=failure;
        OutputSession session; QVERIFY(session.connect(modules,identity(),profile,std::make_unique<Transport>(record)));
        QVERIFY(session.stage(frame(profile))); QVERIFY(!session.flush());
        QCOMPARE(session.state(),OutputSession::State::Faulted); QVERIFY(!session.error().isEmpty());
        QCOMPARE(record->reports.size(),size_t(failure)); QCOMPARE(record->closes,1); QCOMPARE(record->destroyed,1);
        const auto cause=session.error();
        QVERIFY(!session.hasPendingFrame()); QVERIFY(!session.flush()); QVERIFY(!session.stage(frame(profile)));
        QCOMPARE(session.error(),cause);
        QCOMPARE(record->reports.size(),size_t(failure));
        auto fresh=std::make_shared<Recording>();
        QVERIFY(session.connect(modules,identity(),profile,std::make_unique<Transport>(fresh)));
        QVERIFY(fresh->reports.empty()); QVERIFY(session.stage(frame(profile))); QVERIFY(session.flush());
        QCOMPARE(fresh->reports.size(),size_t(3)); // Fresh driver re-arms software and reapplies native settings.
    }
    auto record=std::make_shared<Recording>(); record->throwWrite=true;
    OutputSession throwing; QVERIFY(throwing.connect(modules,identity(),profile,std::make_unique<Transport>(record)));
    QVERIFY(throwing.stage(frame(profile))); QVERIFY(!throwing.flush());
    QCOMPARE(throwing.state(),OutputSession::State::Faulted); QCOMPARE(record->closes,1);
    auto unplugged=std::make_shared<Recording>(); OutputSession session;
    QVERIFY(session.connect(modules,identity(),profile,std::make_unique<Transport>(unplugged)));
    QVERIFY(session.stage(frame(profile))); unplugged->opened=false; QVERIFY(!session.flush());
    QCOMPARE(session.state(),OutputSession::State::Faulted); QVERIFY(unplugged->reports.empty());
}

void OutputTest::lifecycle()
{
    QString error;
    const auto profile=DeviceProfile::load(TEST_PROFILE_PATH);
    auto record=std::make_shared<Recording>();
    {
        ModuleRegistry modules; QVERIFY(modules.load(TEST_MODULE_PATH,&error));
        OutputSession session; QVERIFY(!session.flush());
        auto mismatch=identity(); mismatch.productId=0;
        QVERIFY(!session.connect(modules,mismatch,profile,std::make_unique<Transport>(record)));
        QCOMPARE(record->closes,1); QVERIFY(record->reports.empty());
        auto closed=std::make_shared<Recording>(); closed->opened=false;
        QVERIFY(!session.connect(modules,identity(),profile,std::make_unique<Transport>(closed)));
        QCOMPARE(closed->closes,1); QCOMPARE(closed->opens,0);
        record=std::make_shared<Recording>();
        QVERIFY(session.connect(modules,identity(),profile,std::make_unique<Transport>(record)));
        QVERIFY(session.stage(frame(profile))); // Destructor cancels pending work; it does not restore/off the device.
    }
    QVERIFY(record->reports.empty()); QCOMPARE(record->closes,1); QCOMPARE(record->destroyed,1);
    // DriverInstance retains plugin code even after the registry object goes away.
    OutputSession retained; auto live=std::make_shared<Recording>();
    {
        ModuleRegistry temporary; QVERIFY(temporary.load(TEST_MODULE_PATH,&error));
        QVERIFY(retained.connect(temporary,identity(),profile,std::make_unique<Transport>(live)));
    }
    QVERIFY(retained.stage(frame(profile))); QVERIFY(retained.flush());
}

void OutputTest::documentPreparation()
{
    const auto profile=DeviceProfile::load(TEST_PROFILE_PATH);
    ComponentDefinition component; component.id="strip"; component.name="Two LEDs";
    component.positions={{0,0},{1,0}}; component.mapping={1,0};
    LightingDocument document({{component.id,component}},nullptr,{profile});
    const auto first=document.addComponent(component.id),second=document.addComponent(component.id);
    document.edit("Position",[](LightingState& state) { state.placements[1].origin={160,80}; });
    ChannelAssignment a; a.endpoint=identity().path; a.profileId=profile.id; a.channelId="JARGB1";
    a.software=true; a.placements={first}; document.assignChannel(a);
    auto b=a; b.id=QUuid::createUuid(); b.endpoint="second-endpoint"; b.placements={second}; document.assignChannel(b);
    ChannelAssignment native; native.endpoint=a.endpoint; native.profileId=profile.id; native.channelId="IOShield";
    document.assignChannel(native);
    QImage composed(LightingDocument::canvasSize(),QImage::Format_ARGB32); composed.fill(Qt::transparent);
    composed.setPixelColor(100,80,QColor(200,100,50,128)); composed.setPixelColor(180,80,Qt::green);
    const auto prepared=prepareOutput(document,composed); QCOMPARE(prepared.size(),2);
    QCOMPARE(prepared[0].endpoint,a.endpoint); QCOMPARE(prepared[0].channels.size(),2);
    QCOMPARE(prepared[0].channels[0].colors[0].r,quint8(100));
    QCOMPARE(prepared[0].channels[0].colors[0].g,quint8(50));
    QVERIFY(!prepared[0].channels[1].software); QVERIFY(prepared[0].channels[1].colors.isEmpty());
    QCOMPARE(prepared[1].endpoint,b.endpoint); QCOMPARE(prepared[1].channels[0].colors[0].g,quint8(255));
    QVERIFY_EXCEPTION_THROWN(prepareOutput(document,QImage()),std::invalid_argument);
    document.addLayer();
    document.edit("Masked color",[](LightingState& state) {
        state.layers[0].color=QColor(200,100,50,128); state.layers[0].transparency=50;
    });
    const auto rendered=document.render(); const auto fullPipeline=prepareOutput(document,rendered);
    ModuleRegistry modules; QString error; QVERIFY(modules.load(TEST_MODULE_PATH,&error));
    auto record=std::make_shared<Recording>(); OutputSession session;
    QVERIFY(session.connect(modules,identity(),profile,std::make_unique<Transport>(record)));
    QVERIFY(session.stage(fullPipeline[0])); QVERIFY(session.flush());
    const auto expected=BuildReport0x51(MsiGen1AdvArea::JARGB1,{{50,25,13},{50,25,13}});
    QVERIFY(record->reports[1]==std::vector<std::uint8_t>(expected.begin(),expected.end()));
}

// Exercise the real serializer with a recording transport; never open a physical device.
void OutputTest::animatedFanHeaders()
{
    const auto profile=DeviceProfile::load(TEST_PROFILE_PATH);
    ComponentDefinition fan; fan.id="fan"; fan.name="Test fan";
    fan.positions={{0,0},{1,0}}; fan.mapping={1,0};
    LightingDocument document({{fan.id,fan}},nullptr,{profile});
    for(const auto& header:{QString("JARGB1"),QString("JARGB2")}) {
        ChannelAssignment assignment; assignment.endpoint=identity().path; assignment.profileId=profile.id;
        assignment.channelId=header; assignment.software=true; assignment.placements={document.addComponent(fan.id)};
        document.assignChannel(assignment);
    }
    document.addPaintLayer();
    document.edit("Animated painted coverage",[](auto& state) {
        auto& paint=state.layers[0]; paint.image=QImage(LightingDocument::canvasSize(),QImage::Format_ARGB32_Premultiplied);
        paint.image.fill(Qt::white); paint.cycleRate=1; paint.colorCycle={Qt::red,Qt::blue};
    });
    ModuleRegistry modules; QString error; QVERIFY2(modules.load(TEST_MODULE_PATH,&error),qPrintable(error));
    auto record=std::make_shared<Recording>(); OutputSession session;
    QVERIFY(session.connect(modules,identity(),profile,std::make_unique<Transport>(record)));
    for(const double seconds:{0.0,.5}) {
        const auto frames=prepareOutput(document,document.render(seconds),seconds);
        QCOMPARE(frames.size(),1); QCOMPARE(frames[0].channels.size(),2);
        QVERIFY(session.stage(frames[0])); QVERIFY(session.flush());
        const auto color=seconds==0 ? MsiColor{255,0,0} : MsiColor{0,0,255};
        const auto first=BuildReport0x51(MsiGen1AdvArea::JARGB1,{color,color});
        const auto second=BuildReport0x51(MsiGen1AdvArea::JARGB2,{color,color});
        std::vector<std::vector<std::uint8_t>> colors;
        for(const auto& report:record->reports) if(report[0]==0x51) colors.push_back(report);
        QCOMPARE(colors.size(),seconds==0 ? size_t(2) : size_t(4));
        QVERIFY(colors[colors.size()-2]==std::vector<std::uint8_t>(first.begin(),first.end()));
        QVERIFY(colors.back()==std::vector<std::uint8_t>(second.begin(),second.end()));
    }
}

QTEST_GUILESS_MAIN(OutputTest)
#include "test_output.moc"
