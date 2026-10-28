#include "service/ServiceProtocol.h"
#include "service/ServiceRuntime.h"
#include "service/ServicePanel.h"
#include <QPushButton>
#include <QAction>
#include <QDir>
#include <QGuiApplication>
#include <QDoubleSpinBox>
#include <mutex>
#include <QtTest>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QProcess>
#include <QFile>
#include <QLocalSocket>
#include <QtEndian>
#include <atomic>
#include <future>
#include <memory>
namespace {
QJsonObject cmd(const QString& op) {return {{"version",1},{"op",op}};}
QJsonObject project(const DeviceProfile& profile) {
    ComponentDefinition strip;strip.id="fixture-strip";strip.name="Fixture strip";strip.positions={{0,0},{1,0}};strip.mapping={0,1};
    LightingDocument document({{strip.id,strip}},nullptr,{profile});
    const auto placement=document.addComponent(strip.id);document.addLayer(EffectKind::StaticColor);
    ChannelAssignment assignment;assignment.endpoint="fake-hid-path";assignment.profileId=profile.id;assignment.channelId="JARGB1";assignment.software=true;assignment.placements={placement};document.assignChannel(assignment);
    QTemporaryDir folder;if(!folder.isValid())throw std::runtime_error("fixture directory");const auto path=folder.path()+"/project.json";document.save(path);QFile input(path);if(!input.open(QIODevice::ReadOnly))throw std::runtime_error("fixture read");
    auto result=cmd("project");result["project"]=StudioService::decode(input.readAll());result["enabled"]=true;result["playing"]=true;result["seconds"]=1.;return result;
}
struct Recording {std::atomic_int writes{0};std::atomic_bool present{true};};
class Transport:public IHidDevice {
public:
    explicit Transport(std::shared_ptr<Recording> r):record(std::move(r)){}
    bool open(std::uint16_t,std::uint16_t)override{return false;}
    void close()override{opened=false;}
    bool isOpen()const override{return opened;}
    bool sendFeatureReport(const std::uint8_t*,std::size_t)override{++record->writes;return record->present;}
    std::shared_ptr<Recording> record;bool opened=true;
};
DeviceIdentity identity(){DeviceIdentity d;d.path="fake-hid-path";d.vendorId=0xdb0;d.productId=0x76;d.boardManufacturer="Micro-Star International Co., Ltd.";d.boardModel="MPG B850 EDGE TI WIFI (MS-7E62)";return d;}
struct Child {
    QProcess process;
    ~Child(){process.kill();process.waitForFinished(3000);}
};
}
class ServiceTest:public QObject {
 Q_OBJECT
private slots:
 void protocolBounds();
 void persistedPlaybackAndAtomicReject();
 void fakeOutputPauseAndResume();
 void authenticatedPipe();
 void editorSynchronization();
 void persistedPauseAndReconnect();
 void editorRestoresServiceSnapshot();
};
void ServiceTest::protocolBounds(){
    const auto encoded=StudioService::encode(cmd("status"));QCOMPARE(qFromBigEndian<quint32>(encoded.constData()),quint32(encoded.size()-4));
    QCOMPARE(StudioService::decode(encoded.mid(4)),cmd("status"));
    QVERIFY_EXCEPTION_THROWN(StudioService::decode("[]"),std::invalid_argument);
    QVERIFY_EXCEPTION_THROWN(StudioService::decode("{"),std::invalid_argument);
    QVERIFY_EXCEPTION_THROWN(StudioService::decode(QByteArray(StudioService::maximumMessage+1,'x')),std::invalid_argument);
}
void ServiceTest::persistedPlaybackAndAtomicReject() try {
    QTemporaryDir folder;QVERIFY(folder.isValid());const auto profile=DeviceProfile::load(TEST_PROFILE_PATH);
    const auto scan=[] {return QVector<ProfileCandidate>{};};const auto output=[](const QVector<OutputFrame>&){return QVector<OutputConnection>{};};
    ServiceRuntime runtime({}, {profile},folder.path(),scan,output,true);
    auto request=project(profile);QVERIFY(runtime.command(request).value("ok").toBool());
    auto hold=cmd("playback");hold["playing"]=false;hold["seconds"]=12.5;QVERIFY(runtime.command(hold).value("ok").toBool());
    QFile saved(folder.path()+"/playback.json");QVERIFY(saved.open(QIODevice::ReadOnly));const auto before=saved.readAll();saved.close();
    auto invalid=request;auto bad=request.value("project").toObject();bad["schemaVersion"]=999;invalid["project"]=bad;
    QVERIFY(!runtime.command(invalid).value("ok").toBool());QCOMPARE(runtime.status().value("seconds").toDouble(),12.5);
    QVERIFY(saved.open(QIODevice::ReadOnly));QCOMPARE(saved.readAll(),before);saved.close();
    auto unknown=cmd("execute");unknown["register"]=123;QVERIFY(!runtime.command(unknown).value("ok").toBool());
    auto mismatch=cmd("status");mismatch["version"]=2;QVERIFY(!runtime.command(mismatch).value("ok").toBool());
    ServiceRuntime restored({}, {profile},folder.path(),scan,output,true);
    QCOMPARE(restored.status().value("seconds").toDouble(),12.5);QVERIFY(!restored.status().value("playing").toBool());
    QCOMPARE(restored.status().value("outputs").toArray().size(),1);
    auto wrap=cmd("playback");wrap["playing"]=true;wrap["seconds"]=86399.95;
    QVERIFY(restored.command(wrap).value("ok").toBool());QTest::qWait(100);
    QVERIFY(restored.status().value("seconds").toDouble()<1.0);
    auto snapshot=restored.command(cmd("snapshot"));QCOMPARE(snapshot.value("project"),request.value("project"));
} catch(const std::exception& e){QFAIL(e.what());}
void ServiceTest::fakeOutputPauseAndResume() try {
    QTemporaryDir folder;const auto profile=DeviceProfile::load(TEST_PROFILE_PATH);auto record=std::make_shared<Recording>();
    const auto output=[record,profile](const QVector<OutputFrame>& frames){
        QVector<OutputConnection> result;for(const auto& frame:frames)result.append({frame,[record,profile]{
            ModuleRegistry modules;QString error;if(!modules.load(TEST_MODULE_PATH,&error))throw std::runtime_error(error.toStdString());
            auto session=std::make_unique<OutputSession>();if(!session->connect(modules,identity(),profile,std::make_unique<Transport>(record)))throw std::runtime_error(session->error().toStdString());return session;
        },[record]{return record->present.load();}});return result;
    };
    ServiceRuntime runtime({}, {profile},folder.path(),[]{return QVector<ProfileCandidate>{};},output);
    QVERIFY(runtime.command(project(profile)).value("ok").toBool());QTRY_VERIFY(record->writes>1);
    const auto key=runtime.status().value("outputs").toArray()[0].toObject().value("key");
    auto pause=cmd("channels");pause["keys"]=QJsonArray{key};pause["paused"]=true;QVERIFY(runtime.command(pause).value("ok").toBool());
    QTest::qWait(100);const int count=record->writes;QTest::qWait(130);QCOMPARE(record->writes.load(),count);
    pause["paused"]=false;QVERIFY(runtime.command(pause).value("ok").toBool());QTRY_VERIFY(record->writes>count);
    runtime.suspend(true);const int suspended=record->writes;QTest::qWait(100);QCOMPARE(record->writes.load(),suspended);
    runtime.suspend(false);QTRY_VERIFY(record->writes>suspended);
    runtime.setOutputGuard([]{throw std::runtime_error("Fixture controller conflict");});
    auto blocked=runtime.command(cmd("scan"));QCOMPARE(blocked.value("error").toString(),QString("Fixture controller conflict"));
    const int blockedWrites=record->writes;QTest::qWait(100);QCOMPARE(record->writes.load(),blockedWrites);
    runtime.setOutputGuard({});runtime.command(cmd("scan"));QTRY_VERIFY(record->writes>blockedWrites);
    auto stop=cmd("output");stop["enabled"]=false;QVERIFY(runtime.command(stop).value("ok").toBool());const int stopped=record->writes;QTest::qWait(100);QCOMPARE(record->writes.load(),stopped);
    QVERIFY(!runtime.status().value("enabled").toBool());
} catch(const std::exception& e){QFAIL(e.what());}
void ServiceTest::authenticatedPipe(){
#ifdef Q_OS_WIN
    QTemporaryDir folder;const auto pipe="LightingStudio.Test."+QUuid::createUuid().toString(QUuid::WithoutBraces);
    Child host;host.process.setProgram(SERVICE_HOST_PATH);host.process.setArguments({"--simulate","--state-dir",folder.path(),"--pipe",pipe});host.process.start();QVERIFY(host.process.waitForStarted());
    QJsonObject reply;bool connected=false;
    for(int i=0;i<40 && !connected;++i){try{reply=StudioService::request(cmd("status"),SERVICE_HOST_PATH,true,pipe);connected=true;}catch(const std::exception&){QTest::qWait(50);}}
    QVERIFY2(connected,host.process.readAllStandardError().constData());QVERIFY(reply.value("simulation").toBool());
    const auto profile=DeviceProfile::load(TEST_PROFILE_PATH);
    auto upload=project(profile);auto large=upload.value("project").toObject();large["fixturePadding"]=QString(2*1024*1024,'x');upload["project"]=large;
    QVERIFY(StudioService::request(upload,SERVICE_HOST_PATH,true,pipe).value("ok").toBool());
    QCOMPARE(StudioService::request(cmd("snapshot"),SERVICE_HOST_PATH,true,pipe).value("project").toObject(),large);
    host.process.kill();QVERIFY(host.process.waitForFinished(3000));host.process.start();QVERIFY(host.process.waitForStarted());
    bool restored=false;
    for(int i=0;i<40 && !restored;++i){try{reply=StudioService::request(cmd("snapshot"),SERVICE_HOST_PATH,true,pipe);restored=true;}catch(const std::exception&){QTest::qWait(50);}}
    QVERIFY(restored);QCOMPARE(reply.value("project").toObject(),large);
    QVERIFY_EXCEPTION_THROWN(StudioService::request(cmd("status"),"C:/invalid-host.exe",true,pipe),std::runtime_error);
    QVERIFY_EXCEPTION_THROWN(StudioService::request(cmd("execute"),SERVICE_HOST_PATH,true,pipe),std::runtime_error);
    QLocalSocket malformed;malformed.connectToServer(pipe);QVERIFY(malformed.waitForConnected(1500));
    QByteArray oversized(4,0);qToBigEndian<quint32>(StudioService::maximumMessage+1,oversized.data());malformed.write(oversized);malformed.waitForBytesWritten(1500);QVERIFY(malformed.waitForDisconnected(1500));
    QVERIFY(StudioService::request(cmd("status"),SERVICE_HOST_PATH,true,pipe).value("ok").toBool());
    Child unauthorized;const auto other=pipe+".denied";unauthorized.process.setProgram(SERVICE_HOST_PATH);unauthorized.process.setArguments({"--simulate","--state-dir",folder.path()+"/other","--pipe",other,"--owner","S-1-5-21-1-2-3-9999"});unauthorized.process.start();QVERIFY(unauthorized.process.waitForStarted());QTest::qWait(400);
    QVERIFY_EXCEPTION_THROWN(StudioService::request(cmd("status"),SERVICE_HOST_PATH,true,other),std::runtime_error);
#else
    QSKIP("Windows named-pipe authentication");
#endif
}

void ServiceTest::persistedPauseAndReconnect() {
    QTemporaryDir folder;const auto profile=DeviceProfile::load(TEST_PROFILE_PATH);
    auto record=std::make_shared<Recording>();
    const auto output=[record,profile](const QVector<OutputFrame>& frames){
        QVector<OutputConnection> result;for(const auto& frame:frames)result.append({frame,[record,profile]{
            ModuleRegistry modules;QString error;if(!modules.load(TEST_MODULE_PATH,&error))throw std::runtime_error(error.toStdString());
            auto session=std::make_unique<OutputSession>();if(!session->connect(modules,identity(),profile,std::make_unique<Transport>(record)))throw std::runtime_error(session->error().toStdString());return session;
        },[record]{return record->present.load();}});return result;
    };
    {
        ServiceRuntime runtime({}, {profile},folder.path(),[]{return QVector<ProfileCandidate>{};},output);
        auto request=project(profile);QVERIFY(runtime.command(request).value("ok").toBool());QTRY_VERIFY(record->writes>0);
        record->present=false;QTRY_COMPARE_WITH_TIMEOUT(runtime.status().value("outputs").toArray()[0].toObject().value("state").toString(),QString("Faulted"),2500);
        record->present=true;QVERIFY(runtime.command(cmd("scan")).value("ok").toBool());QTRY_COMPARE(runtime.status().value("outputs").toArray()[0].toObject().value("state").toString(),QString("Running"));
        auto pause=cmd("channels");pause["keys"]=QJsonArray{runtime.status().value("outputs").toArray()[0].toObject().value("key")};pause["paused"]=true;QVERIFY(runtime.command(pause).value("ok").toBool());
        // Updating effects must retain channel pause state and persisted locks.
        QVERIFY(runtime.command(request).value("ok").toBool());QCOMPARE(runtime.status().value("outputs").toArray()[0].toObject().value("state").toString(),QString("Paused"));
    }
    ServiceRuntime restored({}, {profile},folder.path(),[]{return QVector<ProfileCandidate>{};},output);
    QCOMPARE(restored.status().value("outputs").toArray()[0].toObject().value("state").toString(),QString("Paused"));
}
void ServiceTest::editorSynchronization() {
    const auto profile=DeviceProfile::load(TEST_PROFILE_PATH);LightingWindow window({},[]{return QVector<ProfileCandidate>{};},nullptr,{profile});
    struct State{std::mutex mutex;QList<QJsonObject> requests;};auto state=std::make_shared<State>();
    ServicePanel panel(window,[state](const QJsonObject& request){
        std::lock_guard<std::mutex> lock(state->mutex);state->requests.append(request);
        return QJsonObject{{"ok",true},{"enabled",false},{"playing",false},{"seconds",0.},{"devices",QJsonArray{}},{"outputs",QJsonArray{}}};
    });window.installOutputPanel(&panel);panel.activate();QTest::qWait(200);
    window.document().addEmptyLayer();
    const auto count=[state](const QString& op){std::lock_guard<std::mutex> lock(state->mutex);int count=0;for(const auto& r:state->requests)if(r.value("op")==op)++count;return count;};
    QTRY_VERIFY(count("project")>0);
    {
        std::lock_guard<std::mutex> lock(state->mutex);
        for(const auto& request:state->requests)if(request.value("op")=="project")QVERIFY(!request.value("enabled").toBool());
    }
    auto* playback=window.findChild<QPushButton*>("playPreview");playback->setChecked(!playback->isChecked());QTRY_VERIFY(count("playback")>0);
    window.findChild<QAction*>("workspaceHardware")->trigger();window.resize(1440,900);window.show();QTest::qWait(50);
    QDir().mkpath(TEST_SERVICE_ARTIFACT_DIR);QVERIFY(window.grab().save(QString(TEST_SERVICE_ARTIFACT_DIR)+"/service-hardware-"+QGuiApplication::platformName()+".png"));
    window.document().addEmptyLayer();const int before=count("project");
    // A close immediately after an edit must synchronously confirm the last project.
    window.closing();QVERIFY(count("project")>before);
}

void ServiceTest::editorRestoresServiceSnapshot() {
    const auto profile=DeviceProfile::load(TEST_PROFILE_PATH);LightingWindow window({},[]{return QVector<ProfileCandidate>{};},nullptr,{profile});
    QTemporaryDir files;window.document().addEmptyLayer();window.saveProject(files.path()+"/old-project.json");
    QFile original(window.projectFile());QVERIFY(original.open(QIODevice::ReadOnly));const auto oldBytes=original.readAll();original.close();
    const auto remote=project(profile).value("project");std::atomic_int uploads{0};
    ServicePanel panel(window,[remote,&uploads](const QJsonObject& request){
        if(request.value("op")=="project")++uploads;
        return QJsonObject{{"ok",true},{"enabled",true},{"playing",false},{"seconds",7.},{"project",remote},{"devices",QJsonArray{}},{"outputs",QJsonArray{}}};
    });panel.activate();
    QTRY_COMPARE(window.document().state().assignments.size(),1);QVERIFY(window.projectFile().isEmpty());
    QCOMPARE(window.previewSeconds(),7.);QVERIFY(window.document().history()->isClean());
    QCOMPARE(uploads.load(),0);QVERIFY(original.open(QIODevice::ReadOnly));QCOMPARE(original.readAll(),oldBytes);
    window.closing();
}
QTEST_MAIN(ServiceTest)
#include "test_service.moc"
