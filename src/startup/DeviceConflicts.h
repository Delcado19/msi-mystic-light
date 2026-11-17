#pragma once
#include "ConflictProcesses.h"
#include <QElapsedTimer>
#include <QHash>
#include <QMutex>
#include <QStringList>
#include <functional>

// Device-based conflict detection (owner decisions 2026-11-16): a device is in conflict only while
// another process sends lighting commands to it, not because a program runs or holds the device open.
// Read-only experiments that day showed: an exclusive-open probe reports a held HID interface; holding
// is not writing (MSI Center's CC_Engine_x64.exe only reads the MSI controller, LEDKeeper2.exe writes
// only on "Apply" and not at all with Mystic Light switched off); a USB trace names the process that
// dispatches each HID SET_REPORT, even SignalRGB, which hides its handles from inspection.
// RAM (SMBus) and GPU endpoints cannot be checked this way.

struct EndpointCheck {
    QString endpoint;
    QVector<ConflictProcess> holders; // Processes seen writing to it, neither Windows nor Lighting Studio.
    QString error;                    // The capture failed; reported, but it never blocks output.
    qint64 checkedAt = 0;             // Epoch ms of the capture.
    bool blocks() const { return !holders.isEmpty(); }
};

// One HID SET_REPORT seen in a USB trace.
struct UsbWrite { quint32 pid = 0; QString device; int interfaceNumber = -1; int count = 0; };

// Policy helpers, unit-tested.
bool isExcludedImage(const QString& path, const QString& systemRoot, const QStringList& ownExecutables);
QString endpointLabel(const QString& endpoint);
// True for a HID SET_REPORT setup packet (8 bytes); interfaceNumber receives wIndex.
bool hidSetReport(const unsigned char* setup, int* interfaceNumber);
// "VID_xxxx&PID_yyyy" from a device path, or empty.
QString usbDeviceId(const QString& path);
// Whether a write to device/interface reaches this HID endpoint.
bool writeTargets(const QString& endpoint, const QString& device, int interfaceNumber);
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
// Body of `lighting_service_host.exe --stop-process <pid> <started>`, run elevated after the user
// approved UAC for exactly this process. Re-reads the image from the live process, refuses Windows
// images and Lighting Studio itself, then stops it like stopHolder(force=true).
QString stopApprovedProcess(quint32 pid, quint64 started);

// Thread-safe; shared by the periodic output guard and the editor's conflict dialog.
class DeviceConflictMonitor {
public:
    // Periodic guard: probes every endpoint; a busy one gets a 5 s USB capture at most once a minute.
    // Returns the endpoints another process writes to; message explains them or a failed capture.
    QStringList blocked(const QStringList& endpoints, QString& message);
    // On demand for the dialog: fresh capture for all busy endpoints; writers plus their program family.
    QVector<ConflictProcess> conflicts(const QStringList& endpoints, QString* error = nullptr);
    // The last conflicts() result; the service stops only processes listed here.
    QVector<ConflictProcess> last() const;
private:
    // The guard and the dialog can ask at the same time; only one USB trace can run. The second caller
    // waits and reuses verdicts younger than 10 s instead of failing (a failed verdict never blocks,
    // so a lost race would let the service connect to a device another program writes to).
    QVector<EndpointCheck> writersFor(const QStringList& endpoints);
    QMutex capture_;
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
