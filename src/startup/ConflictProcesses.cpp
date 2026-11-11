#include "ConflictProcesses.h"
#include <QCoreApplication>
#include <QApplication>
#include <QWidget>
#include <QDir>
#include <QFileInfo>
#include <stdexcept>
#ifdef Q_OS_WIN
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#include <tlhelp32.h>
#include <memory>
#endif

namespace {
QString normalized(QString executable)
{
    return QFileInfo(QDir::fromNativeSeparators(executable)).fileName().toLower();
}
}

QString conflictReason(const QString& executable)
{
    // Exact executable names only: a vendor name does not imply RGB access.
    const auto name = normalized(executable);
    if (name == "signalrgbservice.exe") return "SignalRGB background service may control RAM and other RGB devices";
    if (name == "signalrgb.exe") return "SignalRGB may control the same RGB devices";
    if (name == "openrgb.exe") return "OpenRGB may control the same RGB devices";
    if (name == "icue.exe") return "Corsair iCUE lighting controller";
    if (name == "lghub.exe" || name == "lghub_agent.exe") return "Logitech G HUB lighting controller";
    if (name == "armourycrate.exe" || name == "lightingservice.exe") return "ASUS lighting controller";
    if (name == "steelseriesgg.exe" || name == "steelseriesengine3.exe") return "SteelSeries lighting controller";
    if (name == "ledkeeper2.exe" || name == "mysticlight.exe") return "MSI Mystic Light controller";
    if (name == "nzxt cam.exe") return "NZXT CAM lighting controller";
    return {};
}

bool requiresManualStop(const QString& executable)
{
    const auto name = normalized(executable);
    // Service-owned helpers can restart immediately; do not kill their host.
    return name == "lightingservice.exe" || name == "ledkeeper2.exe";
}

bool isSignalRgbService(const QString& executable)
{
    return normalized(executable) == "signalrgbservice.exe";
}

QString signalRgbAdminStopError(const ConflictService& service,const QVector<ConflictProcess>& processes)
{
    if(service.name.compare("SignalRgb.Service",Qt::CaseInsensitive)!=0 || !service.ownProcess)
        return "Cannot verify the exclusive SignalRGB service";
    if(service.state==ConflictService::Stopped)return "SignalRGB service is already stopped";
    if(service.state==ConflictService::StopPending)return "SignalRGB service is already stopping";
    if(!service.pid || service.state!=ConflictService::Running || !service.acceptsStop)
        return "SignalRGB service does not currently accept stop";
    auto command=service.commandLine.trimmed();QString executable;
    if(command.startsWith('"')) {
        const auto end=command.indexOf('"',1);
        if(end<2 || (end+1<command.size() && !command[end+1].isSpace()))return "Invalid SignalRGB service ImagePath";
        executable=command.mid(1,end-1);
    } else {
        int end=0;while(end<command.size() && !command[end].isSpace())++end;
        executable=command.left(end);
    }
    // Ambiguous unquoted paths, unresolved variables and non-local paths fail closed.
    if(executable.size()<4 || executable[0].toUpper()<'A' || executable[0].toUpper()>'Z' || executable[1]!=':' ||
       (executable[2]!='/' && executable[2]!='\\') || executable.contains('%') || executable.contains('"') ||
       executable.contains(QChar::Null) || !isSignalRgbService(executable))
        return "Cannot verify the local SignalRGB service executable";
    for(const auto& process:processes)if(normalized(process.executable)=="signalrgb.exe")
        return "Close the SignalRGB application before stopping its service; it may restart the service";
    for(const auto& process:processes)if(process.pid==service.pid && isSignalRgbService(process.executable)) {
        // Protected services may deny process metadata. Compare it when available; SCM remains the named-service authority.
        if(!process.path.isEmpty() && QDir::cleanPath(QDir::fromNativeSeparators(process.path)).compare(
            QDir::cleanPath(QDir::fromNativeSeparators(executable)),Qt::CaseInsensitive)!=0)
            return "SignalRGB process path differs from the service configuration; rescan before trying again";
        return {};
    }
    return "SignalRGB service identity changed; rescan before trying again";
}

QString signalRgbAdminStopExitError(quint32 code)
{
    if(code==0 || code==1062)return {}; // A stopped service must still be verified by the caller.
    if(code==1223)return "Administrator service stop cancelled (Windows error 1223)";
    if(code==5)return "Administrator service stop denied (Windows error 5)";
    if(code==1051)return "SignalRGB service has running dependent services (Windows error 1051)";
    return "SignalRGB service stop failed (Windows error "+QString::number(code)+")";
}

QString stopSignalRgbService(const ConflictProcess& process, bool verifyOnly, IConflictServices& services)
{
    if (!isSignalRgbService(process.executable) || !process.pid ||
        !matchesProcessIdentity(process, process.started, process.path))
        return "Cannot verify SignalRGB service identity";
    QVector<ConflictService> list;
    auto error = services.enumerate(list);
    if (!error.isEmpty()) return error;
    QVector<ConflictService> owners;
    for (const auto& service : list) if (service.pid == process.pid) owners.append(service);
    if (owners.size() != 1 || !owners[0].ownProcess)
        return "Cannot verify exclusive own-process service ownership; stop SignalRGB manually";
    ConflictService current;
    error = services.query(owners[0].name, current);
    if (!error.isEmpty()) return error;
    if (current.state == ConflictService::Stopped) return {};
    if (current.pid != process.pid || !current.ownProcess)
        return "Service identity changed; rescan before trying again";
    if (current.state == ConflictService::StopPending) return {};
    if (verifyOnly) return "SignalRGB service is not stopping; retry or stop it manually";
    if (current.state != ConflictService::Running || !current.acceptsStop)
        return "SignalRGB service does not currently accept stop; stop it manually";
    return services.requestStop(current.name);
}

bool matchesProcessIdentity(const ConflictProcess& expected, quint64 started, const QString& path)
{
    return expected.started != 0 && !expected.path.isEmpty() && expected.started == started &&
        normalized(expected.path) == normalized(expected.executable) &&
        expected.path.compare(path, Qt::CaseInsensitive) == 0;
}

#ifdef Q_OS_WIN
namespace {
using Handle = std::unique_ptr<void, decltype(&CloseHandle)>;
quint64 creationTime(HANDLE process)
{
    FILETIME created{}, exited{}, kernel{}, user{};
    if (!GetProcessTimes(process, &created, &exited, &kernel, &user)) return 0;
    return (quint64(created.dwHighDateTime) << 32) | created.dwLowDateTime;
}
QString processPath(HANDLE process)
{
    wchar_t buffer[32768];
    DWORD length = 32768;
    return QueryFullProcessImageNameW(process, 0, buffer, &length)
        ? QString::fromWCharArray(buffer, static_cast<int>(length)) : QString{};
}
QString winError(const QString& action)
{
    return action + " (Windows error " + QString::number(GetLastError()) + ")";
}
struct ScCloser { void operator()(SC_HANDLE h) const { if (h) CloseServiceHandle(h); } };
using ScHandle = std::unique_ptr<std::remove_pointer_t<SC_HANDLE>, ScCloser>;
ConflictService serviceInfo(const QString& name, const SERVICE_STATUS_PROCESS& s)
{
    ConflictService result;
    result.name = name; result.pid = s.dwProcessId;
    result.ownProcess = s.dwServiceType == SERVICE_WIN32_OWN_PROCESS;
    result.acceptsStop = (s.dwControlsAccepted & SERVICE_ACCEPT_STOP) != 0;
    result.state = s.dwCurrentState == SERVICE_STOPPED ? ConflictService::Stopped :
        s.dwCurrentState == SERVICE_RUNNING ? ConflictService::Running :
        s.dwCurrentState == SERVICE_STOP_PENDING ? ConflictService::StopPending : ConflictService::Other;
    return result;
}
class NativeServices final : public IConflictServices {
    quint32 verifiedPid_ = 0;
    ScHandle manager_{OpenSCManagerW(nullptr, nullptr, SC_MANAGER_ENUMERATE_SERVICE | SC_MANAGER_CONNECT)};
public:
    QString enumerate(QVector<ConflictService>& result) override {
        if (!manager_) return winError("Cannot access service manager; run as administrator");
        DWORD needed = 0, count = 0, resume = 0;
        EnumServicesStatusExW(manager_.get(), SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
            nullptr, 0, &needed, &count, &resume, nullptr);
        if (GetLastError() != ERROR_MORE_DATA) return winError("Cannot enumerate service ownership");
        QVector<BYTE> data(needed); resume = 0;
        if (!EnumServicesStatusExW(manager_.get(), SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
            data.data(), DWORD(data.size()), &needed, &count, &resume, nullptr))
            return winError("Cannot enumerate complete service ownership");
        auto* entries = reinterpret_cast<ENUM_SERVICE_STATUS_PROCESSW*>(data.data());
        for (DWORD i = 0; i < count; ++i)
            result.append(serviceInfo(QString::fromWCharArray(entries[i].lpServiceName), entries[i].ServiceStatusProcess));
        return {};
    }
    QString query(const QString& name, ConflictService& result) override {
        ScHandle service(OpenServiceW(manager_.get(), reinterpret_cast<LPCWSTR>(name.utf16()), SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG));
        if (!service) return winError("Cannot query SignalRGB service; run as administrator");
        SERVICE_STATUS_PROCESS status{}; DWORD needed = 0;
        if (!QueryServiceStatusEx(service.get(), SC_STATUS_PROCESS_INFO, reinterpret_cast<BYTE*>(&status), sizeof(status), &needed))
            return winError("Cannot query SignalRGB service state");
        result = serviceInfo(name, status); verifiedPid_ = result.pid;
        DWORD bytes=0;QueryServiceConfigW(service.get(),nullptr,0,&bytes);
        if(GetLastError()!=ERROR_INSUFFICIENT_BUFFER || !bytes || bytes>1024*1024)return winError("Cannot size SignalRGB service configuration");
        QVector<BYTE> buffer(bytes);
        auto* config=reinterpret_cast<QUERY_SERVICE_CONFIGW*>(buffer.data());
        if(!QueryServiceConfigW(service.get(),config,bytes,&bytes))return winError("Cannot read SignalRGB service configuration");
        const auto expanded=ExpandEnvironmentStringsW(config->lpBinaryPathName,nullptr,0);
        if(!expanded || expanded>32768)return "Invalid SignalRGB service ImagePath length";
        QVector<wchar_t> path(expanded);
        const auto length=ExpandEnvironmentStringsW(config->lpBinaryPathName,path.data(),expanded);
        if(!length || length>expanded)return winError("Cannot expand SignalRGB service ImagePath");
        result.commandLine=QString::fromWCharArray(path.data());return {};
    }
    QString requestStop(const QString& name) override {
        ScHandle service(OpenServiceW(manager_.get(), reinterpret_cast<LPCWSTR>(name.utf16()), SERVICE_STOP | SERVICE_QUERY_STATUS));
        if (!service) return winError("Cannot stop SignalRGB service; run as administrator");
        SERVICE_STATUS_PROCESS current{}; DWORD needed = 0;
        if (!QueryServiceStatusEx(service.get(), SC_STATUS_PROCESS_INFO, reinterpret_cast<BYTE*>(&current), sizeof(current), &needed))
            return winError("Cannot reverify SignalRGB service");
        if (current.dwProcessId != verifiedPid_ || current.dwServiceType != SERVICE_WIN32_OWN_PROCESS)
            return "Service identity changed; rescan before trying again";
        if (current.dwCurrentState == SERVICE_STOP_PENDING || current.dwCurrentState == SERVICE_STOPPED) return {};
        SERVICE_STATUS status{};
        if (!ControlService(service.get(), SERVICE_CONTROL_STOP, &status))
            return winError("SignalRGB service stop failed; retry or stop manually");
        return {};
    }
};
BOOL CALLBACK closeWindow(HWND window, LPARAM pid)
{
    DWORD owner = 0;
    GetWindowThreadProcessId(window, &owner);
    if (owner == static_cast<DWORD>(pid) && GetWindow(window, GW_OWNER) == nullptr)
        PostMessageW(window, WM_CLOSE, 0, 0);
    return TRUE;
}
}
#endif

SystemConflictProcesses::~SystemConflictProcesses()
{
#ifdef Q_OS_WIN
    if(adminStopProcess_)CloseHandle(adminStopProcess_);
#endif
}

QString SystemConflictProcesses::querySignalRgbService(ConflictService& service)
{
#ifdef Q_OS_WIN
    NativeServices services;return services.query("SignalRgb.Service",service);
#else
    Q_UNUSED(service);return "SignalRGB service stop is available on Windows only";
#endif
}

QString SystemConflictProcesses::requestSignalRgbAdminStop(quint32 expectedPid)
{
#ifdef Q_OS_WIN
    if(!expectedPid)return "SignalRGB service identity changed; rescan before trying again";
    bool pending=false;const auto previous=pollSignalRgbAdminStop(pending);
    if(pending)return "An administrator service-stop request is already active";
    if(!previous.isEmpty())return "Previous administrator stop request failed: "+previous+". Retry to send a new request";
    ConflictService service;const auto error=querySignalRgbService(service);if(!error.isEmpty())return error;
    if(!expectedPid || service.pid!=expectedPid)return "SignalRGB service identity changed; rescan before trying again";
    const auto identityError=signalRgbAdminStopError(service,scan());if(!identityError.isEmpty())return identityError;
    wchar_t systemDirectory[MAX_PATH];const auto size=GetSystemDirectoryW(systemDirectory,MAX_PATH);
    if(!size || size>=MAX_PATH)return winError("Cannot locate Windows service controller");
    const auto executable=QString::fromWCharArray(systemDirectory)+"\\sc.exe";
    // A user-requested named-service stop, never a shell/PID command or privileged editor launch.
    SHELLEXECUTEINFOW launch{};launch.cbSize=sizeof(launch);
    launch.fMask=SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
    if(auto* owner=QApplication::activeModalWidget())launch.hwnd=reinterpret_cast<HWND>(owner->winId());
    launch.lpVerb=L"runas";launch.lpFile=reinterpret_cast<LPCWSTR>(executable.utf16());
    launch.lpParameters=L"stop SignalRgb.Service";launch.nShow=SW_HIDE;
    if(!ShellExecuteExW(&launch)) {
        const auto code=GetLastError();const auto message=signalRgbAdminStopExitError(code);
        return message.isEmpty()?"Cannot launch administrator service-stop request (Windows error "+QString::number(code)+")":message;
    }
    if(!launch.hProcess)return "Cannot track the administrator service-stop request; rescan service state";
    adminStopProcess_=launch.hProcess;return {};
#else
    Q_UNUSED(expectedPid);return "SignalRGB service stop is available on Windows only";
#endif
}

QString SystemConflictProcesses::pollSignalRgbAdminStop(bool& pending)
{
    pending=false;
#ifdef Q_OS_WIN
    if(!adminStopProcess_)return {};
    const auto wait=WaitForSingleObject(adminStopProcess_,0);
    if(wait==WAIT_TIMEOUT){pending=true;return {};}
    DWORD code=0;QString error;
    if(wait!=WAIT_OBJECT_0 || !GetExitCodeProcess(adminStopProcess_,&code))error=winError("Cannot read administrator service-stop result");
    else error=signalRgbAdminStopExitError(code);
    CloseHandle(adminStopProcess_);adminStopProcess_=nullptr;return error;
#else
    return "SignalRGB service stop is available on Windows only";
#endif
}

QVector<ConflictProcess> SystemConflictProcesses::scan()
{
#ifdef Q_OS_WIN
    Handle snapshot(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0), CloseHandle);
    if (snapshot.get() == INVALID_HANDLE_VALUE) {
        snapshot.release();
        throw std::runtime_error(winError("Cannot list running processes").toStdString());
    }
    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);
    QVector<ConflictProcess> result;
    BOOL next = Process32FirstW(snapshot.get(), &entry);
    while (next) {
        const auto executable = QString::fromWCharArray(entry.szExeFile);
        const auto reason = conflictReason(executable);
        if (!reason.isEmpty() && entry.th32ProcessID != QCoreApplication::applicationPid()) {
            ConflictProcess process;
            process.pid = entry.th32ProcessID;
            process.executable = executable;
            process.reason = reason;
            process.manualStop = requiresManualStop(executable);
            Handle handle(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, process.pid), CloseHandle);
            if (handle) {
                process.path = processPath(handle.get());
                process.started = creationTime(handle.get());
            }
            result.append(process);
        }
        next = Process32NextW(snapshot.get(), &entry);
    }
    if (GetLastError() != ERROR_NO_MORE_FILES)
        throw std::runtime_error(winError("Process enumeration incomplete").toStdString());
    return result;
#else
    throw std::runtime_error("Process conflict detection is currently supported on Windows only");
#endif
}

QString SystemConflictProcesses::stop(const ConflictProcess& process, bool force)
{
    if (process.pid == 0 || process.pid == QCoreApplication::applicationPid() ||
        conflictReason(process.executable).isEmpty()) return "Refusing an invalid conflict target";
    if (requiresManualStop(process.executable))
        return "Stop the owning lighting application/service manually; service settings are not changed";
    if (!process.started || process.path.isEmpty() || normalized(process.path) != normalized(process.executable))
        return isSignalRgbService(process.executable) ? "Cannot verify process identity; use Stop SignalRGB service (administrator)" : "Cannot verify process identity; stop it manually";
#ifdef Q_OS_WIN
    const DWORD rights = PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE | (force && !isSignalRgbService(process.executable) ? PROCESS_TERMINATE : 0);
    Handle handle(OpenProcess(rights, FALSE, process.pid), CloseHandle);
    if (!handle) {
        if (GetLastError() == ERROR_INVALID_PARAMETER) return {};
        return winError("Cannot access process");
    }
    if (WaitForSingleObject(handle.get(), 0) == WAIT_OBJECT_0) return {};
    if (!matchesProcessIdentity(process, creationTime(handle.get()), processPath(handle.get())))
        return "Process identity changed; rescan before trying again";
    if (isSignalRgbService(process.executable)) {
        NativeServices services;
        return stopSignalRgbService(process, force, services);
    }
    if (force) {
        // The checked handle, not a fresh PID lookup, is the termination target.
        if (!TerminateProcess(handle.get(), 1)) return winError("Cannot terminate process");
    } else {
        EnumWindows(closeWindow, static_cast<LPARAM>(process.pid));
    }
    return {};
#else
    Q_UNUSED(force);
    return "Process termination is currently supported on Windows only";
#endif
}
