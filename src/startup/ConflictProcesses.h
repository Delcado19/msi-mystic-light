#pragma once

#include <QString>
#include <QVector>

struct ConflictProcess {
    quint32 pid = 0;
    QString executable, path, reason;
    quint64 started = 0; // OS creation time protects against PID reuse.
    bool manualStop = false;
};

QString conflictReason(const QString& executable);
bool requiresManualStop(const QString& executable);
// Compare OS handle metadata against the snapshot before any close/terminate action.
bool matchesProcessIdentity(const ConflictProcess& expected, quint64 started, const QString& path);
struct ConflictService;

class IConflictProcesses {
public:
    virtual ~IConflictProcesses() = default;
    virtual QVector<ConflictProcess> scan() = 0;
    // Empty string means requested/already exited; callers must rescan.
    virtual QString stop(const ConflictProcess& process, bool force) = 0;
    // Separate, explicit named-service action; normal process termination never elevates.
    virtual QString querySignalRgbService(ConflictService&) { return "SignalRGB service query unavailable"; }
    // Must reject another request while its controller process is still pending, including after a UI timeout.
    virtual QString requestSignalRgbAdminStop(quint32) { return "Administrator service stop unavailable"; }
    virtual QString pollSignalRgbAdminStop(bool& pending) { pending=false;return {}; }
};

class SystemConflictProcesses final : public IConflictProcesses {
public:
    SystemConflictProcesses() = default;
    SystemConflictProcesses(const SystemConflictProcesses&) = delete;
    SystemConflictProcesses& operator=(const SystemConflictProcesses&) = delete;
    ~SystemConflictProcesses() override;
    QVector<ConflictProcess> scan() override;
    QString stop(const ConflictProcess& process, bool force) override;
    QString querySignalRgbService(ConflictService& service) override;
    QString requestSignalRgbAdminStop(quint32 expectedPid) override;
    QString pollSignalRgbAdminStop(bool& pending) override;
private:
    void* adminStopProcess_ = nullptr;
};

// Platform-neutral SCM policy seam. Values mirror only the states we need.
struct ConflictService {
    QString name;
    quint32 pid = 0;
    bool ownProcess = false;
    enum State { Stopped, Running, StopPending, Other } state = Other;
    bool acceptsStop = false;
    QString commandLine; // Expanded SCM ImagePath, never executed by the application.
};
class IConflictServices {
public:
    virtual ~IConflictServices() = default;
    virtual QString enumerate(QVector<ConflictService>& services) = 0;
    virtual QString query(const QString& name, ConflictService& service) = 0;
    virtual QString requestStop(const QString& name) = 0;
};
// Caller must hold and verify the original process handle before invoking this.
QString stopSignalRgbService(const ConflictProcess& process, bool verifyOnly, IConflictServices& services);
bool isSignalRgbService(const QString& executable);
// Validates the fixed SCM service independently of inaccessible process-token metadata.
QString signalRgbAdminStopError(const ConflictService& service,const QVector<ConflictProcess>& processes);
QString signalRgbAdminStopExitError(quint32 exitCode);
