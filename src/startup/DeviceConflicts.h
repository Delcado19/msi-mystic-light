#pragma once
#include "ConflictProcesses.h"
#include <QElapsedTimer>
#include <QHash>
#include <QMutex>
#include <QStringList>
#include <functional>

// Device-based conflict detection (owner decision 2026-11-16): find the process that holds one of our
// lighting endpoints instead of blocking on program names. Read-only experiments that day showed:
// an exclusive-open probe reports a held HID interface even when its holder protects its handles
// (SignalRGB); a system handle snapshot names unprotected holders (MSI Center's CC_Engine_x64.exe);
// a held interface is not proof of colour writes. RAM (SMBus) and GPU endpoints cannot be probed.

struct EndpointCheck {
    QString endpoint;
    QVector<ConflictProcess> holders; // Visible holders that are neither Windows nor Lighting Studio.
    bool systemOnly = false;          // Only Windows holds it (e.g. hidserv on consumer-control collections).
    bool hidden = false;              // Busy without any visible holder: the holder hides its handles.
    QString error;                    // Holder lookup failed; the endpoint is treated as blocked.
    bool blocks() const { return !holders.isEmpty() || hidden || !error.isEmpty(); }
};

// Policy helpers, unit-tested.
bool isExcludedImage(const QString& path, const QString& systemRoot, const QStringList& ownExecutables);
QString endpointLabel(const QString& endpoint);
// Endpoints with a scheme (pawnio://, nvapi://) are not device files and cannot be probed.
QStringList probeableEndpoints(const QStringList& endpoints);

// The holder's program family (owner request 2026-11-16: stop "mother and child processes" too).
// Walks up while the parent lives in the same install folder or above it within one program folder
// (SignalRgbLauncher.exe in ...\app-2.5.86\ starts ...\app-2.5.86\Signal-x64\SignalRgb.exe), is older
// than its child (PID reuse) and is not excluded; then adds every descendant inside that folder.
// The topmost member comes first, so a launcher is stopped before it can restart what follows.
struct ProcessEntry { quint32 pid = 0, parent = 0; quint64 started = 0; QString path; };
QVector<quint32> processFamily(const QVector<ProcessEntry>& processes, quint32 holder,
    const std::function<bool(const QString&)>& excluded);

// True while another handle to the device exists. Opens the device exclusively for an instant.
bool endpointBusy(const QString& endpoint);

// Stops a detected holder. Known lighting programs keep their dedicated paths (SignalRGB service and
// Mystic_Light_Service via the service manager); any other holder is stopped through the service
// manager when it hosts its own service, otherwise terminated. force=false only asks windows to close.
QString stopHolder(const ConflictProcess& process, bool force);
// Polite close request from the user's session (the service in session 0 cannot reach its windows).
QString requestClose(const ConflictProcess& process);

// Thread-safe; shared by the periodic output guard and the editor's conflict dialog.
class DeviceConflictMonitor {
public:
    // Periodic guard: probes every endpoint, looks up holders only for endpoints that became busy
    // since the last call (the lookup takes seconds). Returns the endpoints that must not be opened.
    QStringList blocked(const QStringList& endpoints, QString& message);
    // On demand for the dialog: fresh holder lookup for all busy endpoints. Holders hiding their
    // handles are offered as possible holders, labelled with the known-program reason if any.
    QVector<ConflictProcess> conflicts(const QStringList& endpoints);
    // The last conflicts() result; the service stops only processes listed here.
    QVector<ConflictProcess> last() const;
private:
    mutable QMutex mutex_;
    QHash<QString, EndpointCheck> busy_; // Verdict per endpoint, kept while it stays busy.
    QVector<ConflictProcess> last_;
    QStringList lastBusy_;
    QElapsedTimer lastTime_;
};

// True when the process runs as ownerSid without elevation. Stopping it grants the requester nothing
// they could not do themselves.
bool runsAsPlainUser(const ConflictProcess& process, const QString& ownerSid);
// Privilege boundary of the LocalSystem service (security review 2026-11-16): any process of the
// owner's account can send stop requests. Suspected holders and family members may therefore be
// stopped only when they run as the owner without elevation; anything more privileged needs proof
// (it was seen holding our device) or must be a known lighting program with its verified stop path.
QString stopAllowed(const ConflictProcess& process, bool plainUser);

// Adapter so stopListedConflict() validates stop requests against the monitor's last result.
class DetectedConflicts final : public IConflictProcesses {
public:
    DetectedConflicts(QVector<ConflictProcess> list, QString ownerSid) : list_(std::move(list)), owner_(std::move(ownerSid)) {}
    QVector<ConflictProcess> scan() override { return list_; }
    QString stop(const ConflictProcess& process, bool force) override {
        const auto refusal = stopAllowed(process, runsAsPlainUser(process, owner_));
        return refusal.isEmpty() ? stopHolder(process, force) : refusal;
    }
    bool stopsServices() const override { return true; }
private:
    QVector<ConflictProcess> list_;
    QString owner_;
};
