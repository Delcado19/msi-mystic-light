#include "service/ServiceProtocol.h"
#include "service/ServiceRuntime.h"
#include "service/ServicePanel.h"
#include <QPushButton>
#include <QLabel>
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
#include <QElapsedTimer>
#include <atomic>
#include <future>
#include <memory>
#include <thread>
#ifdef Q_OS_WIN
#define NOMINMAX
#include <windows.h>
#include <aclapi.h>
#include <sddl.h>
#endif
namespace {
QJsonObject cmd(const QString& op) {return {{"version",1},{"op",op}};}
QJsonObject project(const DeviceProfile& profile, bool secondEndpoint=false) {
    ComponentDefinition strip;strip.id="fixture-strip";strip.name="Fixture strip";strip.positions={{0,0},{1,0}};strip.mapping={0,1};
    LightingDocument document({{strip.id,strip}},nullptr,{profile});
    const auto placement=document.addComponent(strip.id);document.addLayer(EffectKind::StaticColor);
    ChannelAssignment assignment;assignment.endpoint="fake-hid-path";assignment.profileId=profile.id;assignment.channelId="JARGB1";assignment.software=true;assignment.placements={placement};document.assignChannel(assignment);
    if(secondEndpoint) {
        assignment.id=QUuid::createUuid();assignment.endpoint="second-hid-path";
        assignment.placements={document.addComponent(strip.id)};document.assignChannel(assignment);
    }
    QTemporaryDir folder;if(!folder.isValid())throw std::runtime_error("fixture directory");const auto path=folder.path()+"/project.json";document.save(path);QFile input(path);if(!input.open(QIODevice::ReadOnly))throw std::runtime_error("fixture read");
    auto result=cmd("project");result["project"]=StudioService::decode(input.readAll());result["enabled"]=true;result["playing"]=true;result["seconds"]=1.;return result;
}
struct Recording {std::atomic_int writes{0},opens{0};std::atomic_bool present{true};};
class Transport:public IHidDevice {
public:
    explicit Transport(std::shared_ptr<Recording> r):record(std::move(r)){++record->opens;}
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
 void identifyCommand();
 void fakeOutputPauseAndResume();
 void backgroundScanKeepsAnswering();
 void conflictsThroughService();
 void authenticatedPipe();
 void editorSynchronization();
 void persistedPauseAndReconnect();
 void offlineEndpointKeepsHealthyOutput();
 void hungDriverCallKeepsAnswering();
 void reopenReinitializesOutputs();
 void unprobedEndpointStaysClosed();
 void hungScanIsAbandoned();
 void editorRestoresServiceSnapshot();
 void identifyKeepsQueuedEditorState();
};
void ServiceTest::protocolBounds(){
    const auto encoded=StudioService::encode(cmd("status"));QCOMPARE(qFromBigEndian<quint32>(encoded.constData()),quint32(encoded.size()-4));
    QCOMPARE(StudioService::decode(encoded.mid(4)),cmd("status"));
    QVERIFY_EXCEPTION_THROWN(StudioService::decode("[]"),std::invalid_argument);
    QVERIFY_EXCEPTION_THROWN(StudioService::decode("{"),std::invalid_argument);
    QVERIFY_EXCEPTION_THROWN(StudioService::decode(QByteArray(StudioService::maximumMessage+1,'x')),std::invalid_argument);
}
void ServiceTest::identifyCommand() try {
    QTemporaryDir folder;QVERIFY(folder.isValid());const auto profile=DeviceProfile::load(TEST_PROFILE_PATH);
    ServiceRuntime runtime({}, {profile},folder.path(),[]{return QVector<ProfileCandidate>{};},[](const QVector<OutputFrame>&){return QVector<OutputConnection>{};},true);
    const auto request=project(profile);QVERIFY(runtime.command(request).value("ok").toBool());
    const auto id=request.value("project").toObject().value("placements").toArray()[0].toObject().value("id").toString();
    auto identify=cmd("identify");identify["placement"]=id;QVERIFY(runtime.command(identify).value("ok").toBool());
    // Transient: the stored project snapshot does not change.
    QCOMPARE(runtime.command(cmd("snapshot")).value("project"),request.value("project"));
    identify["placement"]=QUuid::createUuid().toString();QVERIFY(!runtime.command(identify).value("ok").toBool());
    identify["placement"]="not-a-uuid";QVERIFY(!runtime.command(identify).value("ok").toBool());
    identify.remove("placement");QVERIFY(!runtime.command(identify).value("ok").toBool());
} catch(const std::exception& e){QFAIL(e.what());}
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
    // The fixture endpoint is discovered, so the device guard may probe it.
    ServiceRuntime runtime({}, {profile},folder.path(),[profile]{return QVector<ProfileCandidate>{{identity(),profile.id}};},output);
    QVERIFY(runtime.command(project(profile)).value("ok").toBool());QTRY_VERIFY(record->writes>1);
    const auto key=runtime.status().value("outputs").toArray()[0].toObject().value("key");
    auto pause=cmd("channels");pause["keys"]=QJsonArray{key};pause["paused"]=true;QVERIFY(runtime.command(pause).value("ok").toBool());
    QTest::qWait(100);const int count=record->writes;QTest::qWait(130);QCOMPARE(record->writes.load(),count);
    pause["paused"]=false;QVERIFY(runtime.command(pause).value("ok").toBool());QTRY_VERIFY(record->writes>count);
    runtime.suspend(true);const int suspended=record->writes;QTest::qWait(100);QCOMPARE(record->writes.load(),suspended);
    runtime.suspend(false);QTRY_VERIFY(record->writes>suspended);
    // Device-based guard (2026-11-16): an endpoint this service holds is never probed and keeps its output.
    QStringList probed{"unset"};
    runtime.setOutputGuard([&probed](const QStringList& targets,QString&){probed=targets;return QStringList{};});
    QVERIFY(runtime.command(cmd("scan")).value("ok").toBool());QVERIFY(probed.isEmpty());
    // A free endpoint is probed; while another program holds it, it stays closed and is explained.
    auto off=cmd("output");off["enabled"]=false;QVERIFY(runtime.command(off).value("ok").toBool());
    runtime.setOutputGuard([](const QStringList& targets,QString& message){message="Fixture controller conflict";return targets;});
    auto on=cmd("output");on["enabled"]=true;const auto blocked=runtime.command(on);
    QCOMPARE(blocked.value("error").toString(),QString("Fixture controller conflict"));
    const int blockedWrites=record->writes;QTest::qWait(100);QCOMPARE(record->writes.load(),blockedWrites);
    runtime.setOutputGuard({});runtime.command(cmd("scan"));QTRY_VERIFY(record->writes>blockedWrites);
    auto stop=cmd("output");stop["enabled"]=false;QVERIFY(runtime.command(stop).value("ok").toBool());const int stopped=record->writes;QTest::qWait(100);QCOMPARE(record->writes.load(),stopped);
    QVERIFY(!runtime.status().value("enabled").toBool());
} catch(const std::exception& e){QFAIL(e.what());}
// Slow hardware discovery must not make the service deaf (owner report 2026-11-16: the editor's
// 1.5 s connect timed out while a ~6.5 s scan ran on the pipe thread).
void ServiceTest::backgroundScanKeepsAnswering() try {
    QTemporaryDir folder;QVERIFY(folder.isValid());const auto profile=DeviceProfile::load(TEST_PROFILE_PATH);
    auto hold=std::make_shared<std::atomic_bool>(true);auto scans=std::make_shared<std::atomic_int>(0);
    // The first scan, started by the constructor, blocks like slow discovery.
    const auto scan=[hold,scans] {
        if(scans->fetch_add(1)==0)while(hold->load())std::this_thread::sleep_for(std::chrono::milliseconds(1));
        return QVector<ProfileCandidate>{};
    };
    ServiceRuntime runtime({}, {profile},folder.path(),scan,[](const QVector<OutputFrame>&){return QVector<OutputConnection>{};});
    struct Release {std::shared_ptr<std::atomic_bool> hold;~Release(){hold->store(false);}} release{hold};   // Never leave the worker blocked.
    runtime.setBackgroundScan(true);QTRY_COMPARE(scans->load(),1);
    bool status=false;runtime.command(cmd("status"),[&status](const QJsonObject& reply){status=reply.value("ok").toBool();});
    QVERIFY(status);
    // A scan command is answered by a scan that started after it, not by the one already running.
    QJsonObject scanned;runtime.command(cmd("scan"),[&scanned](const QJsonObject& reply){scanned=reply;});
    QTest::qWait(50);QVERIFY(scanned.isEmpty());
    hold->store(false);QTRY_VERIFY(!scanned.isEmpty());
    QCOMPARE(scans->load(),2);QVERIFY(scanned.value("ok").toBool());
} catch(const std::exception& e){QFAIL(e.what());}
// Editor side of the service-backed conflict dialog: the 64-bit creation time must survive JSON.
void ServiceTest::conflictsThroughService() {
    QList<QJsonObject> sent;
    ServiceConflictProcesses processes([&sent](const QJsonObject& request)->QJsonObject {
        sent.append(request);
        if(request.value("op")=="conflicts")return {{"ok",true},{"conflicts",QJsonArray{QJsonObject{{"pid",4242},{"started","133000000000000001"},
            {"executable","LEDKeeper2.exe"},{"path","C:/fixture/LEDKeeper2.exe"},{"reason","fixture"},{"manualStop",false}}}}};
        return {{"ok",true},{"stopError","Fixture refusal"}};
    });
    const auto list=processes.scan();QCOMPARE(list.size(),1);
    QCOMPARE(list[0].pid,quint32(4242));QCOMPARE(list[0].started,quint64(133000000000000001ULL));QCOMPARE(list[0].executable,QString("LEDKeeper2.exe"));
    // Forced only: the non-forced step also posts a local close request to the real PID.
    QCOMPARE(processes.stop(list[0],true),QString("Fixture refusal"));
    QCOMPARE(sent.last().value("op").toString(),QString("stopConflict"));QCOMPARE(sent.last().value("pid").toInt(),4242);
    QCOMPARE(sent.last().value("started").toString(),QString("133000000000000001"));QVERIFY(sent.last().value("force").toBool());
    // Administrator stop: the editor runs the host with exactly this PID and creation time.
    QStringList launched;quint32 exit=3;QString launchError;
    ServiceConflictProcesses elevated([](const QJsonObject&)->QJsonObject{return {{"ok",true},{"conflicts",QJsonArray{QJsonObject{
        {"pid",4243},{"started","133000000000000002"},{"executable","Launcher.exe"},{"path","C:/fixture/Launcher.exe"},{"reason","fixture"},{"elevatedStop",true}}}}};},
        [&](const QStringList& arguments,quint32& code){launched=arguments;code=exit;return launchError;});
    const auto approved=elevated.scan();QVERIFY(approved[0].elevatedStop);QVERIFY(!list[0].elevatedStop);
    QVERIFY(elevated.stopElevated(approved[0]).contains("identity changed"));
    QCOMPARE(launched,(QStringList{"--stop-process","4243","133000000000000002"}));
    exit=0;QCOMPARE(elevated.stopElevated(approved[0]),QString());
    launchError="Administrator approval was declined";QCOMPARE(elevated.stopElevated(approved[0]),launchError);
    QVERIFY(!elevated.stopElevated(list[0]).isEmpty()); // Not marked: never elevated.
#ifdef Q_OS_WIN
    // The helper itself rejects malformed and system-reserved PIDs before touching any process.
    QCOMPARE(QProcess::execute(SERVICE_HOST_PATH,{"--stop-process","not-a-pid","1"}),2);
    QCOMPARE(QProcess::execute(SERVICE_HOST_PATH,{"--stop-process","4","1"}),1);
#endif
    ServiceConflictProcesses offline([](const QJsonObject&)->QJsonObject{throw std::runtime_error("Lighting service unavailable");});
    QVERIFY_EXCEPTION_THROWN(offline.scan(),std::runtime_error);QCOMPARE(offline.stop(list[0],true),QString("Lighting service unavailable"));
}
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
    {
        // The pipe DACL admits only SYSTEM, the server account and the owner: no Everyone/Users entries.
        QLocalSocket probe;probe.connectToServer(pipe);QVERIFY(probe.waitForConnected(1500));
        PACL dacl=nullptr;PSECURITY_DESCRIPTOR descriptor=nullptr;
        QCOMPARE(GetSecurityInfo(reinterpret_cast<HANDLE>(probe.socketDescriptor()),SE_KERNEL_OBJECT,DACL_SECURITY_INFORMATION,nullptr,nullptr,&dacl,nullptr,&descriptor),DWORD(ERROR_SUCCESS));
        LPWSTR text=nullptr;QVERIFY(ConvertSecurityDescriptorToStringSecurityDescriptorW(descriptor,SDDL_REVISION_1,DACL_SECURITY_INFORMATION,&text,nullptr));
        const auto sddl=QString::fromWCharArray(text);LocalFree(text);
        QVERIFY2(sddl.startsWith("D:P"),qPrintable(sddl));
        for(const auto* broad:{";;;WD)",";;;AU)",";;;BU)",";;;AN)"})QVERIFY2(!sddl.contains(broad),qPrintable(sddl));
        // Compare SIDs, not SDDL text: Windows prints well-known accounts as aliases (CI runs as LA).
        PSID owner=nullptr;QVERIFY(ConvertStringSidToSidW(reinterpret_cast<LPCWSTR>(StudioService::currentUserSid().utf16()),&owner));
        bool ownerListed=false;
        for(DWORD i=0;i<dacl->AceCount;++i){void* ace=nullptr;if(GetAce(dacl,i,&ace) && static_cast<ACE_HEADER*>(ace)->AceType==ACCESS_ALLOWED_ACE_TYPE && EqualSid(&static_cast<ACCESS_ALLOWED_ACE*>(ace)->SidStart,owner))ownerListed=true;}
        LocalFree(owner);LocalFree(descriptor);
        QVERIFY2(ownerListed,qPrintable(sddl));
        // A second host cannot take over a pipe name that is already served (pipe squatting).
        Child squatter;squatter.process.setProgram(SERVICE_HOST_PATH);squatter.process.setArguments({"--simulate","--state-dir",folder.path()+"/squat","--pipe",pipe});
        squatter.process.start();QVERIFY(squatter.process.waitForFinished(10000));QVERIFY(squatter.process.exitCode()!=0);
        QVERIFY2(squatter.process.readAllStandardError().contains("Cannot bind service pipe"),"squatter failed for another reason");
    }
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
// A scan hung in the driver is given up at its deadline: callers get a reply with an error, no
// second scan piles up while it hangs, and scanning resumes once it returns (CodeRabbit 2026-11-17).
void ServiceTest::hungScanIsAbandoned() try {
    QTemporaryDir folder;QVERIFY(folder.isValid());const auto profile=DeviceProfile::load(TEST_PROFILE_PATH);
    auto hold=std::make_shared<std::atomic_bool>(true);auto scans=std::make_shared<std::atomic_int>(0);
    const auto scan=[hold,scans] {
        if(scans->fetch_add(1)==0)while(hold->load())std::this_thread::sleep_for(std::chrono::milliseconds(1));
        return QVector<ProfileCandidate>{};
    };
    struct Release {std::shared_ptr<std::atomic_bool> hold;~Release(){hold->store(false);}} release{hold};
    ServiceRuntime runtime({}, {profile},folder.path(),scan,[](const QVector<OutputFrame>&){return QVector<OutputConnection>{};});
    runtime.setBackgroundScan(true);runtime.setScanDeadline(300);QTRY_COMPARE(scans->load(),1);
    QJsonObject first;runtime.command(cmd("scan"),[&first](const QJsonObject& reply){first=reply;});
    QTRY_VERIFY_WITH_TIMEOUT(!first.isEmpty(),3000);
    QVERIFY(first.value("error").toString().contains("did not finish"));
    QJsonObject second;runtime.command(cmd("scan"),[&second](const QJsonObject& reply){second=reply;});
    QTRY_VERIFY_WITH_TIMEOUT(!second.isEmpty(),3000);QCOMPARE(scans->load(),1); // No second stuck worker.
    hold->store(false);QTest::qWait(50);
    QJsonObject third;runtime.command(cmd("scan"),[&third](const QJsonObject& reply){third=reply;});
    QTRY_VERIFY_WITH_TIMEOUT(!third.isEmpty(),3000);QVERIFY(scans->load()>=2);
} catch(const std::exception& e){QFAIL(e.what());}
// With a conflict guard, an endpoint is opened only after a scan probed it for other writers
// (CodeRabbit review 2026-11-16: endpoints added or freed during a scan were opened unchecked).
void ServiceTest::unprobedEndpointStaysClosed() try {
    QTemporaryDir folder;const auto profile=DeviceProfile::load(TEST_PROFILE_PATH);auto record=std::make_shared<Recording>();
    auto discovered=std::make_shared<std::atomic_bool>(false);
    const auto output=[record,profile](const QVector<OutputFrame>& frames){
        QVector<OutputConnection> result;for(const auto& frame:frames)result.append({frame,[record,profile]{
            ModuleRegistry modules;QString error;if(!modules.load(TEST_MODULE_PATH,&error))throw std::runtime_error(error.toStdString());
            auto session=std::make_unique<OutputSession>();if(!session->connect(modules,identity(),profile,std::make_unique<Transport>(record)))throw std::runtime_error(session->error().toStdString());return session;
        },[record]{return record->present.load();}});return result;
    };
    const auto scan=[discovered,profile]{return discovered->load()?QVector<ProfileCandidate>{{identity(),profile.id}}:QVector<ProfileCandidate>{};};
    ServiceRuntime runtime({}, {profile},folder.path(),scan,output);
    QStringList probed;runtime.setOutputGuard([&probed](const QStringList& targets,QString&){probed=targets;return QStringList{};});
    QVERIFY(runtime.command(project(profile)).value("ok").toBool());
    QVERIFY(probed.isEmpty());QTest::qWait(100);QCOMPARE(record->opens.load(),0); // Not discovered, so not probed.
    discovered->store(true);QVERIFY(runtime.command(cmd("scan")).value("ok").toBool());
    QCOMPARE(probed,QStringList{"fake-hid-path"});QTRY_COMPARE(record->opens.load(),1);
} catch(const std::exception& e){QFAIL(e.what());}
// After another program was stopped, every output is opened (and so initialized) again once,
// however many stops arrive (owner report 2026-11-17: the Vulcan flickered after Terminate all).
void ServiceTest::reopenReinitializesOutputs() try {
    QTemporaryDir folder;const auto profile=DeviceProfile::load(TEST_PROFILE_PATH);auto record=std::make_shared<Recording>();
    const auto output=[record,profile](const QVector<OutputFrame>& frames){
        QVector<OutputConnection> result;for(const auto& frame:frames)result.append({frame,[record,profile]{
            ModuleRegistry modules;QString error;if(!modules.load(TEST_MODULE_PATH,&error))throw std::runtime_error(error.toStdString());
            auto session=std::make_unique<OutputSession>();if(!session->connect(modules,identity(),profile,std::make_unique<Transport>(record)))throw std::runtime_error(session->error().toStdString());return session;
        },[record]{return record->present.load();}});return result;
    };
    ServiceRuntime runtime({}, {profile},folder.path(),[profile]{return QVector<ProfileCandidate>{{identity(),profile.id}};},output);
    QVERIFY(runtime.command(project(profile)).value("ok").toBool());QTRY_VERIFY(record->writes>1);
    QCOMPARE(record->opens.load(),1);
    QVERIFY(runtime.command(cmd("reopen")).value("ok").toBool());runtime.reopenOutputs();
    QTRY_COMPARE_WITH_TIMEOUT(record->opens.load(),2,4000);
    const int writes=record->writes;QTRY_VERIFY(record->writes>writes);
    QTest::qWait(2000);QCOMPARE(record->opens.load(),2); // Two requests, one reopen.
} catch(const std::exception& e){QFAIL(e.what());}
// A driver call that never returns must not make the service deaf (dump 2026-11-17: the main
// thread joined a worker stuck in HidD_GetSerialNumberString on the MSI controller).
void ServiceTest::hungDriverCallKeepsAnswering() try {
    QTemporaryDir folder;QVERIFY(folder.isValid());const auto profile=DeviceProfile::load(TEST_PROFILE_PATH);
    auto hold=std::make_shared<std::atomic_bool>(true);auto opens=std::make_shared<std::atomic_int>(0);
    struct Release {std::shared_ptr<std::atomic_bool> hold;~Release(){hold->store(false);}} release{hold};
    const auto output=[hold,opens](const QVector<OutputFrame>& frames) {
        QVector<OutputConnection> result;
        for(const auto& frame:frames)result.append({frame,[hold,opens]()->std::unique_ptr<OutputSession> {
            opens->fetch_add(1);while(hold->load())std::this_thread::sleep_for(std::chrono::milliseconds(1));
            return nullptr;
        },{}});
        return result;
    };
    ServiceRuntime runtime({}, {profile},folder.path(),[]{return QVector<ProfileCandidate>{};},output);
    QVERIFY(runtime.command(project(profile)).value("ok").toBool());QTRY_COMPARE(opens->load(),1);
    // Reloading the project stops the hung worker; the command returns within its bounded wait.
    QElapsedTimer timer;timer.start();
    QVERIFY(runtime.command(project(profile)).value("ok").toBool());
    QVERIFY2(timer.elapsed()<5000,"project command blocked on the hung worker");
    // A worker that hung in an earlier call is not waited for again on the next sync.
    timer.restart();QVERIFY(runtime.command(project(profile)).value("ok").toBool());
    QVERIFY2(timer.elapsed()<300,"project command waited again for an earlier hung worker");
    for(int i=0;i<3;++i)QVERIFY(runtime.command(cmd("scan")).value("ok").toBool());
    QCOMPARE(opens->load(),1); // No second stuck worker on the same endpoint.
    QCOMPARE(runtime.status().value("outputs").toArray()[0].toObject().value("state").toString(),QString("Not responding"));
    // Once the call returns, the old worker is reaped and the endpoint is opened again.
    hold->store(false);QTest::qWait(50);
    QVERIFY(runtime.command(cmd("scan")).value("ok").toBool());QTRY_COMPARE(opens->load(),2);
    QVERIFY(runtime.status().value("outputs").toArray()[0].toObject().value("state").toString()!="Not responding");
} catch(const std::exception& e){QFAIL(e.what());}
void ServiceTest::offlineEndpointKeepsHealthyOutput() {
    QTemporaryDir folder;const auto profile=DeviceProfile::load(TEST_PROFILE_PATH);
    auto first=std::make_shared<Recording>(),second=std::make_shared<Recording>();
    bool firstAvailable=false,secondAvailable=false;QVector<OutputFrame> requested;
    const auto output=[&](const QVector<OutputFrame>& frames) {
        requested=frames;QVector<OutputConnection> result;
        for(const auto& frame:frames) {
            const bool isSecond=frame.endpoint=="second-hid-path";
            if(isSecond?!secondAvailable:!firstAvailable)continue;
            auto record=isSecond?second:first;auto device=identity();device.path=frame.endpoint;
            result.append({frame,[record,device,profile] {
                ModuleRegistry modules;QString error;if(!modules.load(TEST_MODULE_PATH,&error))throw std::runtime_error(error.toStdString());
                auto session=std::make_unique<OutputSession>();
                if(!session->connect(modules,device,profile,std::make_unique<Transport>(record)))throw std::runtime_error(session->error().toStdString());
                return session;
            },[record]{return record->present.load();}});
        }
        return result;
    };
    ServiceRuntime runtime({}, {profile},folder.path(),[]{return QVector<ProfileCandidate>{};},output);
    QVERIFY(runtime.command(project(profile,true)).value("ok").toBool());
    QVERIFY(runtime.command(cmd("scan")).value("ok").toBool());
    QCOMPARE(first->opens.load(),0);QCOMPARE(second->opens.load(),0);
    QCOMPARE(runtime.status().value("outputs").toArray()[0].toObject().value("state").toString(),QString("Offline"));
    firstAvailable=true;runtime.command(cmd("scan"));QTRY_VERIFY(first->writes>0);
    for(int i=0;i<3;++i)QVERIFY(runtime.command(cmd("scan")).value("ok").toBool());
    QCOMPARE(first->opens.load(),1);QCOMPARE(requested.size(),1);QCOMPARE(requested[0].endpoint,QString("second-hid-path"));
    QCOMPARE(runtime.status().value("outputs").toArray()[1].toObject().value("state").toString(),QString("Offline"));
    const int writes=first->writes;QTRY_VERIFY(first->writes>writes);
    secondAvailable=true;runtime.command(cmd("scan"));QTRY_VERIFY(second->writes>0);
    QCOMPARE(first->opens.load(),1);QCOMPARE(second->opens.load(),1);
    auto invalid=project(profile,true);invalid["seconds"]=-1.;
    QVERIFY(!runtime.command(invalid).value("ok").toBool());
    QCOMPARE(first->opens.load(),1);QCOMPARE(second->opens.load(),1);
    // A fault on B must not reinitialize A either; factories use distinct verified identities.
    second->present=false;QTRY_COMPARE_WITH_TIMEOUT(runtime.status().value("outputs").toArray()[1].toObject().value("state").toString(),QString("Faulted"),2500);
    second->present=true;runtime.command(cmd("scan"));QTRY_COMPARE(second->opens.load(),2);
    QTRY_COMPARE(runtime.status().value("outputs").toArray()[1].toObject().value("state").toString(),QString("Running"));
    QCOMPARE(first->opens.load(),1);
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
// CodeRabbit review 2026-11-15: identify must not re-enable a queued Stop or drop a project the full queue rejected.
void ServiceTest::identifyKeepsQueuedEditorState() {
    const auto profile=DeviceProfile::load(TEST_PROFILE_PATH);LightingWindow window({},[]{return QVector<ProfileCandidate>{};},nullptr,{profile});
    struct State{std::mutex mutex;QList<QJsonObject> requests;};auto state=std::make_shared<State>();
    ServicePanel panel(window,[state](const QJsonObject& request){
        std::lock_guard<std::mutex> lock(state->mutex);state->requests.append(request);
        return QJsonObject{{"ok",true},{"enabled",true},{"playing",false},{"seconds",0.},{"devices",QJsonArray{}},{"outputs",QJsonArray{}}};
    });window.installOutputPanel(&panel);panel.activate();
    // Wait for the processed snapshot, not a fixed delay: identify is dropped until then (CodeRabbit review 2026-11-18).
    QTRY_VERIFY(panel.findChild<QLabel*>("outputStatus")->text().startsWith("Service connected"));
    const auto ops=[state](const QString& op){std::lock_guard<std::mutex> lock(state->mutex);QList<QJsonObject> found;for(const auto& r:state->requests)if(r.value("op")==op)found.append(r);return found;};
    QVERIFY(ops("project").isEmpty());
    // Stop is still queued when identify sends the pending project; the snapshot must keep output off.
    window.document().addEmptyLayer();panel.stopOutput();emit window.identifyRequested(QUuid::createUuid());
    QTRY_COMPARE(ops("identify").size(),1);QVERIFY(!ops("project").first().value("enabled").toBool());
    // With a full queue identify cannot queue the project; it must stay dirty and reach the service later.
    const int before=ops("project").size();window.document().addEmptyLayer();
    for(int i=0;i<16;++i)panel.stopOutput();
    emit window.identifyRequested(QUuid::createUuid());
    QTRY_VERIFY_WITH_TIMEOUT(ops("project").size()>before,5000);
    window.closing();
}
QTEST_MAIN(ServiceTest)
#include "test_service.moc"
