#include "DeviceConflicts.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QMutexLocker>
#include <QRegularExpression>
#include <QSet>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <tuple>
#include <thread>
#include <vector>
#ifdef Q_OS_WIN
#define NOMINMAX
#include <windows.h>
#include <evntrace.h>
#include <evntcons.h>
#include <tdh.h>
#include <tlhelp32.h>
#include <sddl.h>
#endif

bool isExcludedImage(const QString& path, const QString& systemRoot, const QStringList& ownExecutables)
{
    if (path.isEmpty()) return false; // Unknown image: shown to the user rather than silently trusted.
    const auto clean = QDir::cleanPath(QDir::fromNativeSeparators(path));
    for (const auto& own : ownExecutables)
        if (clean.compare(QDir::cleanPath(QDir::fromNativeSeparators(own)), Qt::CaseInsensitive) == 0) return true;
    const auto root = QDir::cleanPath(QDir::fromNativeSeparators(systemRoot));
    return !root.isEmpty() && clean.startsWith(root + "/", Qt::CaseInsensitive);
}

QString endpointLabel(const QString& endpoint)
{
    const auto match = QRegularExpression("VID_[0-9A-F]{4}&PID_[0-9A-F]{4}(&MI_[0-9A-F]{2})?",
        QRegularExpression::CaseInsensitiveOption).match(endpoint);
    return match.hasMatch() ? match.captured(0).toUpper() : endpoint;
}

bool hidSetReport(const unsigned char* setup, int* interfaceNumber)
{
    // bmRequestType 0x21 (class request to an interface), bRequest 0x09 (SET_REPORT); wIndex names the interface.
    if (setup[0] != 0x21 || setup[1] != 0x09) return false;
    if (interfaceNumber) *interfaceNumber = setup[4] | (setup[5] << 8);
    return true;
}

QString usbDeviceId(const QString& path)
{
    const auto match = QRegularExpression("VID_[0-9A-F]{4}&PID_[0-9A-F]{4}", QRegularExpression::CaseInsensitiveOption).match(path);
    return match.hasMatch() ? match.captured(0).toUpper() : QString{};
}

bool writeTargets(const QString& endpoint, const QString& device, int interfaceNumber)
{
    const auto label = endpointLabel(endpoint);
    if (device.isEmpty() || !label.startsWith(device, Qt::CaseInsensitive)) return false;
    // A composite device's endpoint names its interface (MI_03); a single-interface device has none.
    const auto mi = QRegularExpression("&MI_([0-9A-F]{2})").match(label);
    return !mi.hasMatch() || mi.captured(1).toInt(nullptr, 16) == interfaceNumber;
}

QStringList probeableEndpoints(const QStringList& endpoints)
{
    // Endpoints come from the editor's project, and the LocalSystem service opens them. Only HID device
    // interfaces qualify: a UNC or pipe path would make the service authenticate to a foreign server
    // (security review 2026-11-16). pawnio:// and nvapi:// endpoints cannot be probed anyway.
    QStringList result;
    for (const auto& endpoint : endpoints)
        if (endpoint.startsWith("\\\\?\\HID#", Qt::CaseInsensitive) && !endpoint.contains('/') && !result.contains(endpoint)) result.append(endpoint);
    return result;
}

namespace {
QString folderOf(const QString& path) { return QFileInfo(QDir::cleanPath(QDir::fromNativeSeparators(path))).absolutePath(); }
bool within(const QString& folder, const QString& root)
{
    return folder.compare(root, Qt::CaseInsensitive) == 0 || folder.startsWith(root + "/", Qt::CaseInsensitive);
}
}

QString stopAllowed(const ConflictProcess& process, bool plainUser)
{
    if (plainUser || process.provenHolder || !conflictReason(process.executable).isEmpty()) return {};
    return "Runs with higher rights and was not seen holding a lighting device; stop it manually";
}

QVector<quint32> processFamily(const QVector<ProcessEntry>& processes, quint32 holder,
    const std::function<bool(const QString&)>& excluded)
{
    QHash<quint32, ProcessEntry> byPid;
    for (const auto& process : processes) byPid.insert(process.pid, process);
    if (!byPid.contains(holder) || byPid[holder].path.isEmpty()) return {holder};
    auto top = byPid[holder]; auto root = folderOf(top.path);
    for (int depth = 0; depth < 16; ++depth) {
        const auto parent = byPid.value(top.parent);
        if (!parent.pid || parent.pid == top.pid || parent.path.isEmpty() || excluded(parent.path)) break;
        if (!parent.started || parent.started > top.started) break; // Reused PID: not the real parent.
        const auto folder = folderOf(parent.path);
        // A drive or a shared folder such as "C:/Program Files" is never one program's install root.
        if (folder.count('/') < 2 || !within(root, folder)) break;
        top = parent; root = folder;
    }
    QVector<quint32> family{top.pid};
    for (int i = 0; i < family.size(); ++i) for (const auto& child : processes) {
        const auto parent = byPid.value(family[i]);
        if (child.parent != parent.pid || family.contains(child.pid) || child.path.isEmpty() || excluded(child.path)) continue;
        if (child.started < parent.started || !within(folderOf(child.path), root)) continue;
        family.append(child.pid);
    }
    if (!family.contains(holder)) family.append(holder);
    return family;
}

#ifdef Q_OS_WIN
namespace {
struct HandleCloser { void operator()(HANDLE h) const { if (h && h != INVALID_HANDLE_VALUE) CloseHandle(h); } };
using Handle = std::unique_ptr<std::remove_pointer_t<HANDLE>, HandleCloser>;
struct ScCloser { void operator()(SC_HANDLE h) const { if (h) CloseServiceHandle(h); } };
using ScHandle = std::unique_ptr<std::remove_pointer_t<SC_HANDLE>, ScCloser>;

QString winError(const QString& action) { return action + " (Windows error " + QString::number(GetLastError()) + ")"; }
quint64 creationTime(HANDLE process)
{
    FILETIME created{}, exited{}, kernel{}, user{};
    if (!GetProcessTimes(process, &created, &exited, &kernel, &user)) return 0;
    return (quint64(created.dwHighDateTime) << 32) | created.dwLowDateTime;
}
QString imagePath(HANDLE process)
{
    wchar_t buffer[32768]; DWORD length = 32768;
    return QueryFullProcessImageNameW(process, 0, buffer, &length) ? QString::fromWCharArray(buffer, int(length)) : QString{};
}
QString systemRoot()
{
    wchar_t buffer[MAX_PATH]; const auto size = GetWindowsDirectoryW(buffer, MAX_PATH);
    return size && size < MAX_PATH ? QString::fromWCharArray(buffer, int(size)) : QString{};
}
QStringList ownExecutables()
{
    const auto folder = QCoreApplication::applicationDirPath();
    return {folder + "/msi_mystic_light.exe", folder + "/lighting_service_host.exe"};
}
ConflictProcess processInfo(quint32 pid, HANDLE process)
{
    ConflictProcess result; result.pid = pid; result.started = creationTime(process); result.path = imagePath(process);
    result.executable = result.path.isEmpty() ? QString("PID %1").arg(pid) : QFileInfo(result.path).fileName();
    return result;
}

// Undocumented provider events; layouts observed on Windows 11 build 26300 (experiment 2026-11-16):
// USBHUB3 event 6 maps fid_UsbDevice to the device interface path and is logged for every device when
// the provider is enabled. UCX event 53 is the dispatch of a control transfer, raised in the sending
// process, with the 8-byte setup packet at the end of its payload. Completion events run in whatever
// process is on the CPU and are ignored.
// shortcut: unverified on other Windows builds; a capture without device events reports an error.
const GUID ucxProvider{0x36DA592D, 0xE43A, 0x4E28, {0xAF, 0x6F, 0x4B, 0xC5, 0x7C, 0x5A, 0x11, 0xE8}};
const GUID hubProvider{0xAC52AD17, 0xCC01, 0x4F85, {0x8D, 0xF5, 0x4D, 0xCE, 0x43, 0x33, 0xC9, 0x9B}};
bool eventProperty(PEVENT_RECORD record, const wchar_t* name, std::vector<BYTE>& value)
{
    PROPERTY_DATA_DESCRIPTOR descriptor{}; descriptor.PropertyName = ULONGLONG(name); descriptor.ArrayIndex = ULONG_MAX;
    ULONG size = 0;
    if (TdhGetPropertySize(record, 0, nullptr, 1, &descriptor, &size) != ERROR_SUCCESS || !size) return false;
    value.resize(size);
    return TdhGetProperty(record, 0, nullptr, 1, &descriptor, size, value.data()) == ERROR_SUCCESS;
}
quint64 pointerProperty(PEVENT_RECORD record, const wchar_t* name)
{
    std::vector<BYTE> value;
    if (!eventProperty(record, name, value)) return 0;
    if (value.size() == 8) return *reinterpret_cast<const quint64*>(value.data());
    if (value.size() == 4) return *reinterpret_cast<const quint32*>(value.data());
    return 0;
}
struct Capture {
    QHash<quint64, QString> devices;                       // fid_UsbDevice -> VID_xxxx&PID_yyyy
    std::map<std::tuple<quint32, quint64, int>, int> writes; // (process, fid_UsbDevice, interface) -> count
};
// Runs on the single ProcessTrace thread.
void WINAPI onUsbEvent(PEVENT_RECORD record)
{
    auto* capture = static_cast<Capture*>(record->UserContext);
    const auto id = record->EventHeader.EventDescriptor.Id;
    if (IsEqualGUID(record->EventHeader.ProviderId, hubProvider) && id == 6) {
        std::vector<BYTE> path;
        const auto device = pointerProperty(record, L"fid_UsbDevice");
        if (device && eventProperty(record, L"fid_DeviceInterfacePath", path) && path.size() >= 2) {
            path.push_back(0); path.push_back(0);
            const auto identity = usbDeviceId(QString::fromWCharArray(reinterpret_cast<const wchar_t*>(path.data())));
            if (!identity.isEmpty()) capture->devices.insert(device, identity);
        }
    } else if (IsEqualGUID(record->EventHeader.ProviderId, ucxProvider) && id == 53 && record->UserDataLength >= 48) {
        const auto* setup = static_cast<const unsigned char*>(record->UserData) + record->UserDataLength - 8;
        int interfaceNumber = -1;
        if (!hidSetReport(setup, &interfaceNumber)) return;
        if (const auto device = pointerProperty(record, L"fid_UsbDevice"))
            ++capture->writes[{quint32(record->EventHeader.ProcessId), device, interfaceNumber}];
    }
}

// One real-time session for `milliseconds` (LocalSystem or administrator only).
QVector<UsbWrite> captureUsbWrites(int milliseconds, QString& error)
{
    static std::atomic_bool running{false};
    if (running.exchange(true)) { error = "A USB capture is already running"; return {}; }
    struct Reset { ~Reset() { running.store(false); } } reset;
    static const wchar_t name[] = L"LightingStudioUsbWrites";
    std::vector<BYTE> buffer(sizeof(EVENT_TRACE_PROPERTIES) + sizeof(name));
    const auto properties = [&buffer] {
        std::fill(buffer.begin(), buffer.end(), BYTE(0));
        auto* p = reinterpret_cast<EVENT_TRACE_PROPERTIES*>(buffer.data());
        p->Wnode.BufferSize = ULONG(buffer.size()); p->Wnode.Flags = WNODE_FLAG_TRACED_GUID; p->Wnode.ClientContext = 1;
        p->LogFileMode = EVENT_TRACE_REAL_TIME_MODE; p->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
        return p;
    };
    ControlTraceW(0, name, properties(), EVENT_TRACE_CONTROL_STOP); // A session left behind by a crashed service.
    TRACEHANDLE session = 0;
    if (const auto status = StartTraceW(&session, name, properties()); status != ERROR_SUCCESS) {
        error = QString("Cannot start USB trace (Windows error %1)").arg(status); return {};
    }
    Capture capture;
    EVENT_TRACE_LOGFILEW log{};
    log.LoggerName = const_cast<LPWSTR>(name);
    log.ProcessTraceMode = PROCESS_TRACE_MODE_REAL_TIME | PROCESS_TRACE_MODE_EVENT_RECORD;
    log.EventRecordCallback = onUsbEvent; log.Context = &capture;
    TRACEHANDLE consumer = OpenTraceW(&log);
    if (consumer == INVALID_PROCESSTRACE_HANDLE) {
        error = winError("Cannot open USB trace");
        ControlTraceW(session, nullptr, properties(), EVENT_TRACE_CONTROL_STOP); return {};
    }
    std::thread reader([&consumer] { ProcessTrace(&consumer, 1, nullptr, nullptr); });
    // Enabled after the consumer runs, so the device events logged on enable are not lost.
    for (const auto& provider : {hubProvider, ucxProvider})
        EnableTraceEx2(session, &provider, EVENT_CONTROL_CODE_ENABLE_PROVIDER, TRACE_LEVEL_VERBOSE, ~0ULL, 0, 0, nullptr);
    Sleep(DWORD(milliseconds));
    ControlTraceW(session, nullptr, properties(), EVENT_TRACE_CONTROL_STOP);
    CloseTrace(consumer);
    reader.join();
    if (capture.devices.isEmpty()) { error = "USB trace delivered no device events; write detection is unavailable on this Windows build"; return {}; }
    QVector<UsbWrite> result;
    for (const auto& [key, count] : capture.writes) {
        const auto device = capture.devices.value(std::get<1>(key));
        if (!device.isEmpty()) result.append({std::get<0>(key), device, std::get<2>(key), count});
    }
    return result;
}

// Processes that sent lighting commands to these endpoints during one capture.
QVector<EndpointCheck> findWriters(const QStringList& endpoints)
{
    QVector<EndpointCheck> checks;
    const auto now = QDateTime::currentMSecsSinceEpoch();
    for (const auto& endpoint : endpoints) { EndpointCheck check; check.endpoint = endpoint; check.checkedAt = now; checks.append(check); }
    QString error;
    const auto writes = captureUsbWrites(5000, error);
    if (!error.isEmpty()) { for (auto& check : checks) check.error = error; return checks; }
    const auto self = GetCurrentProcessId(); const auto root = systemRoot(); const auto ours = ownExecutables();
    QHash<quint32, ConflictProcess> processes;
    for (const auto& write : writes) {
        if (write.pid == self || write.pid <= 4) continue;
        for (auto& check : checks) {
            if (!writeTargets(check.endpoint, write.device, write.interfaceNumber)) continue;
            if (!processes.contains(write.pid)) {
                Handle handle(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, write.pid));
                ConflictProcess unknown; unknown.pid = write.pid; unknown.executable = QString("PID %1").arg(write.pid);
                processes.insert(write.pid, handle ? processInfo(write.pid, handle.get()) : unknown);
            }
            auto writer = processes.value(write.pid);
            if (isExcludedImage(writer.path, root, ours)) continue;
            if (std::any_of(check.holders.cbegin(), check.holders.cend(), [&](const auto& h) { return h.pid == writer.pid; })) continue;
            writer.provenHolder = true; check.holders.append(writer);
        }
    }
    return checks;
}

QVector<ProcessEntry> runningProcesses()
{
    QVector<ProcessEntry> result;
    Handle snapshot(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
    if (snapshot.get() == INVALID_HANDLE_VALUE) { snapshot.release(); return result; }
    PROCESSENTRY32W entry{}; entry.dwSize = sizeof(entry);
    for (BOOL next = Process32FirstW(snapshot.get(), &entry); next; next = Process32NextW(snapshot.get(), &entry)) {
        ProcessEntry process; process.pid = entry.th32ProcessID; process.parent = entry.th32ParentProcessID;
        Handle handle(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, process.pid));
        if (handle) { process.started = creationTime(handle.get()); process.path = imagePath(handle.get()); }
        result.append(process);
    }
    return result;
}

// Still the same running instance (not exited, PID not reused).
bool alive(const ConflictProcess& process)
{
    Handle handle(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE, FALSE, process.pid));
    return handle && WaitForSingleObject(handle.get(), 0) == WAIT_TIMEOUT && creationTime(handle.get()) == process.started;
}

BOOL CALLBACK closeWindow(HWND window, LPARAM pid)
{
    DWORD owner = 0; GetWindowThreadProcessId(window, &owner);
    if (owner == DWORD(pid) && !GetWindow(window, GW_OWNER)) PostMessageW(window, WM_CLOSE, 0, 0);
    return TRUE;
}
// Opens the process only if it is still the detected instance (PID reuse, replaced image).
QString openVerified(const ConflictProcess& process, DWORD rights, Handle& handle, bool& exited)
{
    exited = false;
    if (!process.pid || !process.started || process.path.isEmpty()) return "Cannot verify process identity; stop it manually";
    handle.reset(OpenProcess(rights | PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE, FALSE, process.pid));
    if (!handle) { if (GetLastError() == ERROR_INVALID_PARAMETER) exited = true; return exited ? QString{} : winError("Cannot access process"); }
    if (WaitForSingleObject(handle.get(), 0) == WAIT_OBJECT_0) { exited = true; return {}; }
    if (creationTime(handle.get()) != process.started || imagePath(handle.get()).compare(process.path, Qt::CaseInsensitive) != 0)
        return "Process identity changed; rescan before trying again";
    return {};
}
// The own-process service hosted by pid, if any. A killed service would be restarted by its recovery
// actions, so it is stopped through the service manager instead.
QString hostedService(quint32 pid, QString& name)
{
    ScHandle manager(OpenSCManagerW(nullptr, nullptr, SC_MANAGER_ENUMERATE_SERVICE | SC_MANAGER_CONNECT));
    if (!manager) return winError("Cannot access service manager");
    DWORD needed = 0, count = 0, resume = 0;
    EnumServicesStatusExW(manager.get(), SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_ACTIVE, nullptr, 0, &needed, &count, &resume, nullptr);
    if (GetLastError() != ERROR_MORE_DATA) return winError("Cannot enumerate services");
    std::vector<BYTE> data(needed); resume = 0;
    if (!EnumServicesStatusExW(manager.get(), SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_ACTIVE, data.data(), DWORD(data.size()), &needed, &count, &resume, nullptr))
        return winError("Cannot enumerate services");
    const auto* entries = reinterpret_cast<const ENUM_SERVICE_STATUS_PROCESSW*>(data.data());
    for (DWORD i = 0; i < count; ++i) if (entries[i].ServiceStatusProcess.dwProcessId == pid) {
        if (entries[i].ServiceStatusProcess.dwServiceType != SERVICE_WIN32_OWN_PROCESS || !name.isEmpty())
            return "The process hosts shared services; stop it manually";
        name = QString::fromWCharArray(entries[i].lpServiceName);
    }
    return {};
}
QString stopService(const QString& name)
{
    ScHandle manager(OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT));
    if (!manager) return winError("Cannot access service manager");
    ScHandle service(OpenServiceW(manager.get(), reinterpret_cast<LPCWSTR>(name.utf16()), SERVICE_STOP | SERVICE_QUERY_STATUS));
    if (!service) return winError("Cannot open service " + name);
    SERVICE_STATUS status{};
    // Stop only; the start type stays unchanged, so the service returns after a reboot (owner decision 2026-11-16).
    if (!ControlService(service.get(), SERVICE_CONTROL_STOP, &status) && GetLastError() != ERROR_SERVICE_NOT_ACTIVE)
        return winError("Cannot stop service " + name);
    return {};
}
}
#endif

bool endpointBusy(const QString& endpoint)
{
#ifdef Q_OS_WIN
    // Share mode 0 fails with ERROR_SHARING_VIOLATION while another handle is open. Read/write access
    // makes keyboard/mouse collections that Windows opens exclusively fail with access denied instead,
    // so they do not count as busy. The handle is closed at once.
    if (probeableEndpoints({endpoint}).isEmpty()) return false;
    HANDLE handle = CreateFileW(reinterpret_cast<LPCWSTR>(endpoint.utf16()), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING,
        SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION, nullptr);
    if (handle != INVALID_HANDLE_VALUE) { CloseHandle(handle); return false; }
    return GetLastError() == ERROR_SHARING_VIOLATION;
#else
    Q_UNUSED(endpoint); return false;
#endif
}

bool runsAsPlainUser(const ConflictProcess& process, const QString& ownerSid)
{
#ifdef Q_OS_WIN
    Handle handle(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, process.pid));
    if (!handle || creationTime(handle.get()) != process.started) return false;
    HANDLE raw = nullptr;
    if (!OpenProcessToken(handle.get(), TOKEN_QUERY, &raw)) return false;
    Handle token(raw);
    TOKEN_ELEVATION elevation{}; DWORD size = 0;
    if (!GetTokenInformation(token.get(), TokenElevation, &elevation, sizeof(elevation), &size) || elevation.TokenIsElevated) return false;
    alignas(8) unsigned char buffer[256];
    if (!GetTokenInformation(token.get(), TokenUser, buffer, sizeof(buffer), &size)) return false;
    LPWSTR sid = nullptr;
    if (!ConvertSidToStringSidW(reinterpret_cast<TOKEN_USER*>(buffer)->User.Sid, &sid)) return false;
    const bool owner = QString::fromWCharArray(sid).compare(ownerSid, Qt::CaseInsensitive) == 0;
    LocalFree(sid);
    return owner;
#else
    Q_UNUSED(process); Q_UNUSED(ownerSid); return false;
#endif
}

QString requestClose(const ConflictProcess& process)
{
#ifdef Q_OS_WIN
    Handle handle; bool exited = false;
    const auto error = openVerified(process, 0, handle, exited);
    if (!error.isEmpty() || exited) return error;
    EnumWindows(closeWindow, LPARAM(process.pid));
    return {};
#else
    Q_UNUSED(process); return "Process control is supported on Windows only";
#endif
}

QString stopHolder(const ConflictProcess& process, bool force)
{
    // Keep the verified paths for known programs: Mystic_Light_Service restarts LEDKeeper2 through Task
    // Scheduler (not a parent process), and the SignalRGB service has its own identity checks.
    if (!conflictReason(process.executable).isEmpty()) { SystemConflictProcesses known; return known.stop(process, force); }
#ifdef Q_OS_WIN
    if (!force) return requestClose(process);
    Handle handle; bool exited = false;
    if (const auto error = openVerified(process, PROCESS_TERMINATE, handle, exited); !error.isEmpty() || exited) return error;
    QString service;
    if (const auto error = hostedService(process.pid, service); !error.isEmpty()) return error;
    if (!service.isEmpty()) return stopService(service);
    if (!TerminateProcess(handle.get(), 1)) return winError("Cannot terminate process");
    return {};
#else
    return "Process control is supported on Windows only";
#endif
}

QStringList DeviceConflictMonitor::blocked(const QStringList& endpoints, QString& message)
{
    QStringList busy;
    for (const auto& endpoint : probeableEndpoints(endpoints)) if (endpointBusy(endpoint)) busy.append(endpoint);
    QStringList stale;
    {
        QMutexLocker lock(&mutex_);
        for (auto it = busy_.begin(); it != busy_.end();) if (!busy.contains(it.key())) it = busy_.erase(it); else ++it;
        // One capture per busy endpoint and minute, not on every five-second retry: a writer that starts
        // later, or one that stopped writing but keeps the device open, is noticed within a minute.
        const auto now = QDateTime::currentMSecsSinceEpoch();
        for (const auto& endpoint : busy)
            if (!busy_.contains(endpoint) || now - busy_.value(endpoint).checkedAt >= 60000) stale.append(endpoint);
    }
#ifdef Q_OS_WIN
    if (!stale.isEmpty()) {
        const auto checks = findWriters(stale);
        QMutexLocker lock(&mutex_);
        for (const auto& check : checks) busy_.insert(check.endpoint, check);
    }
#endif
    QStringList result, reasons, errors;
    QMutexLocker lock(&mutex_);
    for (const auto& endpoint : busy) {
        const auto check = busy_.value(endpoint);
        // Detection that cannot run must not switch the lighting off; it is reported instead.
        if (!check.error.isEmpty() && !errors.contains(check.error)) errors.append(check.error);
        if (!check.blocks()) continue;
        result.append(endpoint);
        QStringList names; for (const auto& holder : check.holders) names.append(holder.executable);
        reasons.append(endpointLabel(endpoint) + ": " + names.join(", "));
    }
    QStringList parts;
    if (!result.isEmpty()) parts.append("Another program sends lighting commands (" + reasons.join("; ") + "). Choose Resolve conflicts to stop it.");
    if (!errors.isEmpty()) parts.append("Conflict detection unavailable: " + errors.join("; "));
    message = parts.join("\n");
    return result;
}

QVector<ConflictProcess> DeviceConflictMonitor::conflicts(const QStringList& endpoints, QString* error)
{
    QStringList busy;
    for (const auto& endpoint : probeableEndpoints(endpoints)) if (endpointBusy(endpoint)) busy.append(endpoint);
    {
        // The dialog rescans every 100 ms while waiting for a stop; reuse a fresh capture while the same
        // devices stay busy. Freed devices change the busy set and end the reuse at once.
        QMutexLocker lock(&mutex_);
        if (busy == lastBusy_ && lastTime_.isValid() && lastTime_.elapsed() < 3000) {
#ifdef Q_OS_WIN
            // A process stopped since the capture must not stay listed, or the dialog reports it as running.
            last_.removeIf([](const ConflictProcess& process) { return !alive(process); });
#endif
            return last_;
        }
    }
    QVector<ConflictProcess> result;
    const auto add = [&result](ConflictProcess process, const QString& reason) {
        for (auto& existing : result) if (existing.pid == process.pid && existing.started == process.started) {
            if (!existing.reason.contains(reason)) existing.reason += "; " + reason;
            return;
        }
        process.reason = reason; result.append(process);
    };
#ifdef Q_OS_WIN
    const auto checks = busy.isEmpty() ? QVector<EndpointCheck>{} : findWriters(busy);
    for (const auto& check : checks) {
        if (!check.error.isEmpty() && error) *error = check.error;
        for (const auto& writer : check.holders) add(writer, "Sends lighting commands to " + endpointLabel(check.endpoint));
    }
    if (!result.isEmpty()) {
        // Add each writer's program family, topmost first, so a launcher is listed and stopped before
        // the program it would restart.
        const auto running = runningProcesses(); const auto root = systemRoot(); const auto ours = ownExecutables();
        const auto excluded = [&](const QString& path) { return isExcludedImage(path, root, ours); };
        const auto direct = result; result.clear();
        for (const auto& writer : direct) for (const auto pid : processFamily(running, writer.pid, excluded)) {
            if (pid == writer.pid) { add(writer, writer.reason); continue; }
            for (const auto& member : running) if (member.pid == pid) {
                ConflictProcess process; process.pid = pid; process.started = member.started; process.path = member.path;
                process.executable = QFileInfo(member.path).fileName();
                add(process, "Same program as " + writer.executable + "; may start it again");
            }
        }
    }
    QMutexLocker lock(&mutex_);
    for (const auto& check : checks) busy_.insert(check.endpoint, check);
#else
    Q_UNUSED(error);
    QMutexLocker lock(&mutex_);
#endif
    last_ = result; lastBusy_ = busy; lastTime_.start();
    return result;
}
QVector<ConflictProcess> DeviceConflictMonitor::last() const
{
    QMutexLocker lock(&mutex_);
    return last_;
}
