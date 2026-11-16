#include "ServiceProtocol.h"
#include "ServiceServer.h"
#include "ServiceRuntime.h"
#include "backends/AuxiliaryHardware.h"
#include "backends/PawnIoRam.h"
#include "startup/ConflictProcesses.h"
#include "startup/DeviceConflicts.h"
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCryptographicHash>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTextStream>
#include <atomic>
#include <QVariant>
#include <stdexcept>
#ifdef Q_OS_WIN
#define NOMINMAX
#include <windows.h>
#include <shlobj.h>
#endif
namespace {
#ifdef Q_OS_WIN
constexpr wchar_t serviceName[]=L"LightingStudio";
SERVICE_STATUS_HANDLE statusHandle=nullptr;
std::atomic_bool stopping{false};
std::atomic<ServiceRuntime*> runtimeInstance{nullptr};
void report(DWORD state,DWORD error=NO_ERROR) {
    SERVICE_STATUS status{};status.dwServiceType=SERVICE_WIN32_OWN_PROCESS;status.dwCurrentState=state;
    status.dwControlsAccepted=state==SERVICE_RUNNING ? SERVICE_ACCEPT_STOP|SERVICE_ACCEPT_SHUTDOWN|SERVICE_ACCEPT_POWEREVENT:0;
    status.dwWin32ExitCode=error;status.dwWaitHint=state==SERVICE_STOP_PENDING || state==SERVICE_START_PENDING?30000:0;
    status.dwServiceSpecificExitCode=error==ERROR_SERVICE_SPECIFIC_ERROR?1:0;
    status.dwCheckPoint=state==SERVICE_START_PENDING || state==SERVICE_STOP_PENDING?1:0;
    if(statusHandle)SetServiceStatus(statusHandle,&status);
}
DWORD WINAPI control(DWORD code,DWORD event,void*,void*) {
    if(code==SERVICE_CONTROL_STOP || code==SERVICE_CONTROL_SHUTDOWN) {
        stopping=true;report(SERVICE_STOP_PENDING);if(qApp)QMetaObject::invokeMethod(qApp,"quit",Qt::QueuedConnection);
    } else if(code==SERVICE_CONTROL_POWEREVENT && qApp) {
        const bool sleep=event==PBT_APMSUSPEND;
        if(sleep || event==PBT_APMRESUMEAUTOMATIC || event==PBT_APMRESUMESUSPEND)
            QMetaObject::invokeMethod(qApp,[sleep]{if(auto* runtime=runtimeInstance.load())runtime->suspend(sleep);},Qt::QueuedConnection);
    }
    return NO_ERROR;
}
QString knownFolder(REFKNOWNFOLDERID id) {
    PWSTR value=nullptr;if(FAILED(SHGetKnownFolderPath(id,0,nullptr,&value)))throw std::runtime_error("Windows folder lookup failed");
    const auto result=QString::fromWCharArray(value);CoTaskMemFree(value);return result;
}
#endif
QJsonObject readJson(const QString& path) {
    QFile file(path);if(!file.open(QIODevice::ReadOnly) || file.size()>StudioService::maximumMessage)throw std::runtime_error(("Cannot read "+path).toStdString());
    return StudioService::decode(file.readAll());
}
void verifyInstall(const QString& root,const QJsonObject& config) {
#ifdef Q_OS_WIN
    if(QDir::cleanPath(root).compare(QDir::cleanPath(knownFolder(FOLDERID_ProgramFiles)+"/Lighting Studio"),Qt::CaseInsensitive)!=0)
        throw std::runtime_error("Service must run from its protected Program Files installation");
#endif
    const auto manifest=config.value("modules").toObject();
    // Keep the protected module allowlist aligned with the installer and portable profile catalog.
    const QStringList expected{"msi_gen1_module.dll","peripherals_module.dll","auxiliary_module.dll",
        "asrock_polychrome_module.dll","lenovo_motherboard_module.dll","hp_omen_module.dll"};
    const auto actual=QDir(root+"/modules").entryList({"*.dll"},QDir::Files);
    if(actual.size()!=expected.size() || manifest.size()!=expected.size())throw std::runtime_error("Unexpected service modules");
    for(const auto& name:expected) {
        QFile file(root+"/modules/"+name);
        if(!actual.contains(name) || !file.open(QIODevice::ReadOnly) ||
            QString::fromLatin1(QCryptographicHash::hash(file.readAll(),QCryptographicHash::Sha256).toHex())!=manifest.value(name).toString())
            throw std::runtime_error("Installed service module integrity check failed");
    }
}
int run(int argc,char** argv,bool managed) {
    // Never inherit user-controlled Qt plugin search paths into the privileged host.
    qunsetenv("QT_PLUGIN_PATH");qunsetenv("QT_QPA_PLATFORM_PLUGIN_PATH");
    QCoreApplication app(argc,argv);app.setApplicationName("Lighting Studio Service");
    const auto root=QCoreApplication::applicationDirPath();QCoreApplication::setLibraryPaths({root});
    QCommandLineParser parser;parser.addHelpOption();
    parser.addOption({"service","Run under the Windows Service Control Manager."});
    parser.addOption({"simulate","Offline test host: never discover or open physical hardware."});
    parser.addOption({"state-dir","Simulation state directory.","directory"});
    parser.addOption({"pipe","Simulation pipe name.","name"});
    parser.addOption({"owner","Simulation authorized SID.","sid"});parser.process(app);
    const bool simulation=parser.isSet("simulate");
    if(managed && simulation)throw std::runtime_error("Simulation is unavailable in service mode");
    if(!managed && (!simulation || StudioService::isSystem()))throw std::runtime_error("Console use requires --simulate and a non-SYSTEM account");
    QJsonObject config;
    QString directory,owner;
    if(managed) {
        config=readJson(root+"/service.json");verifyInstall(root,config);owner=config.value("ownerSid").toString();
#ifdef Q_OS_WIN
        directory=knownFolder(FOLDERID_ProgramData)+"/LightingStudio";
#endif
    } else {directory=parser.value("state-dir");owner=parser.value("owner");if(owner.isEmpty())owner=StudioService::currentUserSid();}
    if(directory.isEmpty() || !owner.startsWith("S-1-"))throw std::runtime_error("Service installation is incomplete");
    ModuleRegistry modules;const auto errors=modules.loadDirectory(root+"/modules");
    if(!errors.isEmpty())throw std::runtime_error(errors.join("\n").toStdString());
    QVector<DeviceProfile> profiles;
    for(const auto& path:QDir(root+"/profiles").entryInfoList({"*.json"},QDir::Files)) {
        auto profile=DeviceProfile::load(path.absoluteFilePath());QString error;
        if(!modules.acceptsProfile(profile,&error))throw std::runtime_error(error.toStdString());profiles.append(profile);
    }
    if(profiles.isEmpty())throw std::runtime_error("Service profiles missing");
    const auto scan=[profiles,simulation] {
        if(simulation)return QVector<ProfileCandidate>{};
        auto candidates=findProfileCandidates(HidApiDevice::enumerate(),readBoardIdentity(),profiles);
        candidates+=scanAuxiliaryHardware(profiles,true);return candidates;
    };
    // Device-based guard (owner decision 2026-11-16): an endpoint stays closed only while another program
    // holds it. A running program name alone no longer blocks output. RAM and GPU endpoints are not probed.
    const auto monitor=std::make_shared<DeviceConflictMonitor>();
    const auto guard=[simulation,monitor](const QStringList& endpoints,QString& message) {
        return simulation?QStringList{}:monitor->blocked(endpoints,message);
    };
    // The runtime applies the guard right before calling this, between two scans.
    const auto authorize=[profiles,root,simulation](const QVector<OutputFrame>& frames) {
        if(simulation)return QVector<OutputConnection>{};
        return authorizeHardwareOutput(nullptr,profiles,frames,root+"/modules");
    };
    ServiceRuntime runtime(loadComponentCatalog(root+"/components"),profiles,directory,scan,authorize,simulation);
    runtime.setOutputGuard(guard);
    // Discovery blocked the pipe thread for seconds, so the editor's 1.5 s connect timed out (2026-11-16).
    runtime.setBackgroundScan(true);
    // Conflict dialog of the installed editor (2026-11-16): the unelevated editor cannot stop services
    // such as Mystic_Light_Service, so the owner had to end LEDKeeper2 in Task Manager. This host can.
    // Stops are for now only (start types and tasks stay untouched) and limited to processes the last
    // detection found holding one of our devices; a client cannot name any other process.
    const auto answer=[](const ServiceServer::Reply& reply,const QJsonObject& result) {
        if(auto* app=QCoreApplication::instance())QMetaObject::invokeMethod(app,[reply,result]{reply(result);},Qt::QueuedConnection);
    };
    const auto conflicts=[simulation,monitor,answer,&runtime](const QJsonObject& request,const ServiceServer::Reply& reply) {
        const auto op=request.value("op").toString();
        if(op!="conflicts" && op!="stopConflict")return false;
        if(request.value("version").toInt()!=StudioService::protocolVersion)throw std::invalid_argument("Unsupported service protocol");
        if(simulation) {
            if(op=="stopConflict")throw std::invalid_argument("Conflict stop is unavailable in simulation");
            reply({{"ok",true},{"conflicts",QJsonArray{}}});return true;
        }
        // Detection takes seconds and service control can block; both run off the pipe thread.
        if(op=="conflicts") {
            std::thread([monitor,answer,reply,targets=runtime.probeTargets()] {
                QJsonArray list;
                // Creation time is a 64-bit FILETIME; a JSON number would lose precision.
                for(const auto& p:monitor->conflicts(targets))list.append(QJsonObject{{"pid",qint64(p.pid)},{"started",QString::number(p.started)},
                    {"executable",p.executable},{"path",p.path},{"reason",p.reason},{"manualStop",p.manualStop}});
                answer(reply,{{"ok",true},{"conflicts",list}});
            }).detach();
            return true;
        }
        const auto pid=quint32(request.value("pid").toInteger());const auto started=request.value("started").toString().toULongLong();
        const bool force=request.value("force").toBool();
        std::thread([monitor,answer,reply,pid,started,force] {
            QString error;
            try {DetectedConflicts detected(monitor->last());error=stopListedConflict(detected,pid,started,force);}
            catch(const std::exception& e) {error=QString::fromUtf8(e.what());}
            answer(reply,{{"ok",true},{"stopError",error}});
        }).detach();
        return true;
    };
    ServiceServer server(owner,[&runtime,conflicts](const QJsonObject& request,const ServiceServer::Reply& reply) {
        if(conflicts(request,reply))return;
        runtime.command(request,[reply](QJsonObject result){if(!pawnIoRamDiscoveryError().isEmpty())result["discoveryError"]=pawnIoRamDiscoveryError();reply(result);});
    });
    const auto pipe=simulation?parser.value("pipe"):StudioService::pipeName();
    if(pipe.isEmpty() || !server.listen(pipe))throw std::runtime_error(("Cannot bind service pipe: "+server.errorString()).toStdString());
#ifdef Q_OS_WIN
    runtimeInstance=&runtime;
#endif
#ifdef Q_OS_WIN
    if(managed){report(SERVICE_RUNNING);if(stopping)QTimer::singleShot(0,&app,&QCoreApplication::quit);}
#endif
    const int result=app.exec();
#ifdef Q_OS_WIN
    runtimeInstance=nullptr;
#endif
    return result;
}
#ifdef Q_OS_WIN
void WINAPI serviceMain(DWORD,wchar_t**) {
    statusHandle=RegisterServiceCtrlHandlerExW(serviceName,control,nullptr);if(!statusHandle)return;
    report(SERVICE_START_PENDING);
    int argc=2;QByteArray executable;
    // applicationFilePath is unavailable before QCoreApplication exists.
    wchar_t path[32768];GetModuleFileNameW(nullptr,path,32768);executable=QString::fromWCharArray(path).toUtf8();
    char argument[]="--service";char* argv[]={executable.data(),argument,nullptr};
    try{run(argc,argv,true);report(SERVICE_STOPPED);}
    catch(const std::exception& e){
        OutputDebugStringA(e.what());
        try {
            QFile log(knownFolder(FOLDERID_ProgramData)+"/LightingStudio/service-error.log");
            if(log.open(QIODevice::WriteOnly|QIODevice::Truncate))log.write(e.what());
        } catch(...) {OutputDebugStringA("Cannot write Lighting Studio service failure log");}
        report(SERVICE_STOPPED,ERROR_SERVICE_SPECIFIC_ERROR);
    }
}
#endif
}
int main(int argc,char** argv) {
#ifdef Q_OS_WIN
    if(argc==2 && QByteArray(argv[1])=="--service") {
        SERVICE_TABLE_ENTRYW table[]={{const_cast<wchar_t*>(serviceName),serviceMain},{nullptr,nullptr}};
        return StartServiceCtrlDispatcherW(table)?0:int(GetLastError());
    }
#endif
    try{return run(argc,argv,false);}catch(const std::exception& e){fprintf(stderr,"Lighting service failed: %s\n",e.what());return 1;}
}
