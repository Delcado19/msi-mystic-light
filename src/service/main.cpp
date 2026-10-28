#include "ServiceProtocol.h"
#include "ServiceServer.h"
#include "ServiceRuntime.h"
#include "backends/AuxiliaryHardware.h"
#include "backends/PawnIoRam.h"
#include "startup/ConflictProcesses.h"
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
    const QStringList expected{"msi_gen1_module.dll","peripherals_module.dll","auxiliary_module.dll"};
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
    const auto guard=[simulation] {
        if(simulation)return;
        // Recheck periodically: another RGB controller can start after boot or user logon.
        SystemConflictProcesses processes;const auto conflicts=processes.scan();
        if(!conflicts.isEmpty()) {
            QStringList names;for(const auto& item:conflicts)names.append(item.executable);
            throw std::runtime_error(("Conflicting lighting software: "+names.join(", ")+". Stop it explicitly, then resume output.").toStdString());
        }
    };
    const auto authorize=[profiles,root,simulation,guard](const QVector<OutputFrame>& frames) {
        if(simulation)return QVector<OutputConnection>{};
        guard();return authorizeHardwareOutput(nullptr,profiles,frames,root+"/modules");
    };
    ServiceRuntime runtime(loadComponentCatalog(root+"/components"),profiles,directory,scan,authorize,simulation);
    runtime.setOutputGuard(guard);
    ServiceServer server(owner,[&runtime](const auto& request){auto reply=runtime.command(request);if(!pawnIoRamDiscoveryError().isEmpty())reply["discoveryError"]=pawnIoRamDiscoveryError();return reply;});
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
