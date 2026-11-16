#include "DeviceConflicts.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QMutexLocker>
#include <QRegularExpression>
#include <QSet>
#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <thread>
#include <vector>
#ifdef Q_OS_WIN
#define NOMINMAX
#include <windows.h>
#include <winternl.h>
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
void enableDebugPrivilege()
{
    // Needed to duplicate handles of services and elevated processes; not assumed to be enabled.
    HANDLE raw = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &raw)) return;
    Handle token(raw);
    TOKEN_PRIVILEGES privileges{}; privileges.PrivilegeCount = 1; privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    if (LookupPrivilegeValueW(nullptr, SE_DEBUG_NAME, &privileges.Privileges[0].Luid))
        AdjustTokenPrivileges(token.get(), FALSE, &privileges, 0, nullptr, nullptr);
}
using QuerySystem = NTSTATUS(NTAPI*)(ULONG, PVOID, ULONG, PULONG);
using QueryObject = NTSTATUS(NTAPI*)(HANDLE, ULONG, PVOID, ULONG, PULONG);
struct HandleEntry { PVOID object; ULONG_PTR pid; ULONG_PTR handle; ULONG access; USHORT backtrace; USHORT type; ULONG attributes; ULONG reserved; };
QString objectName(QueryObject query, HANDLE handle)
{
    alignas(8) unsigned char buffer[4096]; ULONG length = 0;
    if (query(handle, 1 /*ObjectNameInformation*/, buffer, sizeof(buffer), &length) < 0) return {};
    const auto* name = reinterpret_cast<const UNICODE_STRING*>(buffer);
    return name->Buffer ? QString::fromWCharArray(name->Buffer, name->Length / 2) : QString{};
}
ConflictProcess processInfo(quint32 pid, HANDLE process)
{
    ConflictProcess result; result.pid = pid; result.started = creationTime(process); result.path = imagePath(process);
    result.executable = result.path.isEmpty() ? QString("PID %1").arg(pid) : QFileInfo(result.path).fileName();
    return result;
}

// One system handle snapshot (about 2.2 million handles and 2.7 s elevated, measured 2026-11-16).
QVector<EndpointCheck> lookupHolders(const QStringList& endpoints, QVector<ConflictProcess>* hiding)
{
    QVector<EndpointCheck> checks;
    for (const auto& endpoint : endpoints) checks.append({endpoint});
    const auto fail = [&checks](const QString& error) { for (auto& check : checks) check.error = error; return checks; };
    auto* ntdll = GetModuleHandleW(L"ntdll.dll");
    const auto querySystem = reinterpret_cast<QuerySystem>(ntdll ? GetProcAddress(ntdll, "NtQuerySystemInformation") : nullptr);
    const auto queryObject = reinterpret_cast<QueryObject>(ntdll ? GetProcAddress(ntdll, "NtQueryObject") : nullptr);
    if (!querySystem || !queryObject) return fail("Handle inspection unavailable");
    enableDebugPrivilege();
    // Our own handle to each device yields its object name and the File object type index.
    QHash<QString, int> byName; std::vector<Handle> own;
    for (int i = 0; i < checks.size(); ++i) {
        if (probeableEndpoints({checks[i].endpoint}).isEmpty()) { checks[i].error = "Not a HID device path"; continue; }
        Handle device(CreateFileW(reinterpret_cast<LPCWSTR>(checks[i].endpoint.utf16()), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
            SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION, nullptr));
        if (device.get() == INVALID_HANDLE_VALUE) { checks[i].error = winError("Cannot open device for inspection"); device.release(); continue; }
        const auto name = objectName(queryObject, device.get());
        if (name.isEmpty()) { checks[i].error = "Cannot read device object name"; continue; }
        byName.insert(name, i); own.push_back(std::move(device));
    }
    if (own.empty()) return checks;
    std::vector<unsigned char> buffer; ULONG size = 1u << 24, needed = 0; NTSTATUS status = 0;
    do { buffer.resize(size); status = querySystem(64 /*SystemExtendedHandleInformation*/, buffer.data(), size, &needed); size *= 2; }
    while (status == NTSTATUS(0xC0000004L) && size <= (1u << 30));
    if (status < 0) return fail(QString("Handle snapshot failed (0x%1)").arg(quint32(status), 8, 16, QLatin1Char('0')));
    const auto count = *reinterpret_cast<const ULONG_PTR*>(buffer.data());
    const auto* entries = reinterpret_cast<const HandleEntry*>(buffer.data() + 2 * sizeof(ULONG_PTR));
    const auto self = GetCurrentProcessId();
    USHORT fileType = 0;
    for (ULONG_PTR i = 0; i < count; ++i)
        if (entries[i].pid == self && entries[i].handle == reinterpret_cast<ULONG_PTR>(own.front().get())) { fileType = entries[i].type; break; }
    if (!fileType) return fail("Cannot identify the device handle type");
    struct Process { Handle handle; ConflictProcess info; int files = 0, duplicated = 0; bool opened = false; };
    QHash<quint32, std::shared_ptr<Process>> processes;
    QVector<QSet<quint32>> holders(checks.size());
    for (ULONG_PTR i = 0; i < count; ++i) {
        const auto& entry = entries[i];
        if (entry.type != fileType || entry.pid == self) continue;
        const auto pid = quint32(entry.pid);
        auto& process = processes[pid];
        if (!process) {
            process = std::make_shared<Process>();
            process->handle.reset(OpenProcess(PROCESS_DUP_HANDLE | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid));
            process->opened = bool(process->handle);
            if (process->opened) process->info = processInfo(pid, process->handle.get());
        }
        if (!process->opened) continue;
        ++process->files;
        HANDLE copy = nullptr;
        if (!DuplicateHandle(process->handle.get(), reinterpret_cast<HANDLE>(entry.handle), GetCurrentProcess(), &copy, 0, FALSE, DUPLICATE_SAME_ACCESS)) continue;
        Handle duplicate(copy); ++process->duplicated;
        // HID handles report FILE_TYPE_UNKNOWN; disk files, consoles and pipes are skipped before the name
        // query, which can block forever on a synchronous pipe.
        if (GetFileType(duplicate.get()) != FILE_TYPE_UNKNOWN) continue;
        const auto found = byName.constFind(objectName(queryObject, duplicate.get()));
        if (found != byName.constEnd()) holders[found.value()].insert(pid);
    }
    const auto root = systemRoot(); const auto ours = ownExecutables();
    for (int i = 0; i < checks.size(); ++i) {
        if (!checks[i].error.isEmpty()) continue;
        bool system = false;
        for (const auto pid : holders[i]) {
            const auto& info = processes[pid]->info;
            if (isExcludedImage(info.path, root, ours)) system = true;
            else { auto holder = info; holder.provenHolder = true; checks[i].holders.append(holder); }
        }
        checks[i].systemOnly = checks[i].holders.isEmpty() && system;
        checks[i].hidden = checks[i].holders.isEmpty() && !system;
    }
    // shortcut: "openable but no handle duplicable" names candidates for hidden holders; SignalRgb.exe
    // matched this unelevated (all 10108 duplications denied). Unverified under LocalSystem.
    if (hiding) for (const auto& process : processes)
        if (process->opened && process->files > 0 && process->duplicated == 0 && !isExcludedImage(process->info.path, root, ours))
            hiding->append(process->info);
    return checks;
}

// The snapshot or a name query could hang inside a driver; never let that freeze the scan worker.
QVector<EndpointCheck> lookupWithTimeout(const QStringList& endpoints, QVector<ConflictProcess>* hiding)
{
    using Result = std::pair<QVector<EndpointCheck>, QVector<ConflictProcess>>;
    auto task = std::make_shared<std::packaged_task<Result()>>([endpoints] {
        QVector<ConflictProcess> candidates; auto checks = lookupHolders(endpoints, &candidates); return Result{checks, candidates};
    });
    auto future = task->get_future();
    const auto failed = [&endpoints](const QString& error) {
        QVector<EndpointCheck> checks;
        for (const auto& endpoint : endpoints) checks.append({endpoint, {}, false, false, error});
        return checks;
    };
    // At most one lookup thread: a hung one would otherwise be joined by a new one on every scan
    // (CodeRabbit review 2026-11-16). Endpoints stay blocked until the hung lookup returns.
    static std::atomic_bool running{false};
    if (running.exchange(true)) return failed("A previous holder lookup is still running");
    // shortcut: a hung lookup leaks its detached thread; acceptable for a rare driver hang, revisit if seen.
    std::thread([task] { (*task)(); running.store(false); }).detach();
    if (future.wait_for(std::chrono::seconds(20)) != std::future_status::ready) return failed("Holder lookup timed out");
    auto result = future.get();
    if (hiding) *hiding = result.second;
    return result.first;
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
    QStringList unknown;
    {
        QMutexLocker lock(&mutex_);
        for (auto it = busy_.begin(); it != busy_.end();) if (!busy.contains(it.key())) it = busy_.erase(it); else ++it;
        for (const auto& endpoint : busy) if (!busy_.contains(endpoint)) unknown.append(endpoint);
    }
    QHash<QString, EndpointCheck> fresh;
#ifdef Q_OS_WIN
    // Holders are looked up once per busy period, not on every five-second retry. A failed lookup blocks
    // this round but is not cached: a device Windows never releases (hidserv) would stay blocked forever.
    if (!unknown.isEmpty()) {
        const auto checks = lookupWithTimeout(unknown, nullptr);
        QMutexLocker lock(&mutex_);
        for (const auto& check : checks) { fresh.insert(check.endpoint, check); if (check.error.isEmpty()) busy_.insert(check.endpoint, check); }
    }
#endif
    QStringList result, reasons;
    QMutexLocker lock(&mutex_);
    for (const auto& endpoint : busy) {
        const auto check = fresh.contains(endpoint) ? fresh.value(endpoint) : busy_.value(endpoint);
        if (!check.blocks()) continue;
        result.append(endpoint);
        QStringList names; for (const auto& holder : check.holders) names.append(holder.executable);
        reasons.append(endpointLabel(endpoint) + ": " + (!check.error.isEmpty() ? check.error
            : names.isEmpty() ? QString("a program that hides its handles") : names.join(", ")));
    }
    if (!result.isEmpty())
        message = "Lighting devices in use by another program (" + reasons.join("; ") + "). Choose Resolve conflicts to stop it.";
    return result;
}

QVector<ConflictProcess> DeviceConflictMonitor::conflicts(const QStringList& endpoints)
{
    QStringList busy;
    for (const auto& endpoint : probeableEndpoints(endpoints)) if (endpointBusy(endpoint)) busy.append(endpoint);
    {
        // The dialog rescans every 100 ms while waiting for a stop; reuse a fresh lookup while the same
        // devices stay busy. Freed devices change the busy set and end the reuse at once.
        QMutexLocker lock(&mutex_);
        if (busy == lastBusy_ && lastTime_.isValid() && lastTime_.elapsed() < 3000) {
#ifdef Q_OS_WIN
            // A process stopped since the lookup must not stay listed, or the dialog reports it as running.
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
    QVector<ConflictProcess> hiding;
    const auto checks = busy.isEmpty() ? QVector<EndpointCheck>{} : lookupWithTimeout(busy, &hiding);
    QStringList hidden;
    for (const auto& check : checks) {
        for (const auto& holder : check.holders) add(holder, "Holds " + endpointLabel(check.endpoint));
        if (check.hidden) hidden.append(endpointLabel(check.endpoint));
    }
    // The name list only labels a candidate; it never makes a process a conflict by itself.
    if (!hidden.isEmpty()) for (const auto& process : hiding) {
        const auto known = conflictReason(process.executable);
        add(process, "May hold " + hidden.join(", ") + "; hides its handles" + (known.isEmpty() ? QString{} : " (" + known + ")"));
    }
    if (!result.isEmpty()) {
        // Add each holder's program family, topmost first, so a launcher is listed and stopped before
        // the program it would restart.
        const auto running = runningProcesses(); const auto root = systemRoot(); const auto ours = ownExecutables();
        const auto excluded = [&](const QString& path) { return isExcludedImage(path, root, ours); };
        const auto direct = result; result.clear();
        for (const auto& holder : direct) for (const auto pid : processFamily(running, holder.pid, excluded)) {
            if (pid == holder.pid) { add(holder, holder.reason); continue; }
            for (const auto& member : running) if (member.pid == pid) {
                ConflictProcess process; process.pid = pid; process.started = member.started; process.path = member.path;
                process.executable = QFileInfo(member.path).fileName();
                add(process, "Same program as " + holder.executable + "; may start it again");
            }
        }
    }
    QMutexLocker lock(&mutex_);
    for (const auto& check : checks) busy_.insert(check.endpoint, check);
#else
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
