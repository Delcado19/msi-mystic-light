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

class IConflictProcesses {
public:
    virtual ~IConflictProcesses() = default;
    virtual QVector<ConflictProcess> scan() = 0;
    // Empty string means requested/already exited; callers must rescan.
    virtual QString stop(const ConflictProcess& process, bool force) = 0;
};

class SystemConflictProcesses final : public IConflictProcesses {
public:
    QVector<ConflictProcess> scan() override;
    QString stop(const ConflictProcess& process, bool force) override;
};
