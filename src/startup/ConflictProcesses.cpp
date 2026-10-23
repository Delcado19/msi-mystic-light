#include "ConflictProcesses.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <stdexcept>
#ifdef Q_OS_WIN
#define NOMINMAX
#include <windows.h>
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
        return "Cannot verify process identity; stop it manually";
#ifdef Q_OS_WIN
    const DWORD rights = PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE | (force ? PROCESS_TERMINATE : 0);
    Handle handle(OpenProcess(rights, FALSE, process.pid), CloseHandle);
    if (!handle) {
        if (GetLastError() == ERROR_INVALID_PARAMETER) return {};
        return winError("Cannot access process");
    }
    if (WaitForSingleObject(handle.get(), 0) == WAIT_OBJECT_0) return {};
    if (!matchesProcessIdentity(process, creationTime(handle.get()), processPath(handle.get())))
        return "Process identity changed; rescan before trying again";
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
