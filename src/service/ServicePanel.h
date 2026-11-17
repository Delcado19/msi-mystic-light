#pragma once
#include "editor/LightingWindow.h"
#include "startup/DeviceConflicts.h"
#include <QJsonObject>
#include <QLabel>
#include <QTreeWidget>
#include <QTimer>
#include <future>
// Installed editors never open physical devices; the service owns playback across GUI exit.
class ServicePanel : public QWidget {
public:
    using Rpc=std::function<QJsonObject(const QJsonObject&)>;
    explicit ServicePanel(LightingWindow& window,Rpc rpc = {});
    void activate(bool preferLocal = false);
    void stopOutput();
private:
    void poll();
    bool send(QJsonObject command); // false when the queue is full and the command was dropped
    QJsonObject project();
    void display(const QJsonObject& status);
    void flushOnClose();
    Rpc rpc_;
    LightingWindow& window_;
    QLabel* status_;
    QTreeWidget* outputs_;
    QTimer timer_;
    std::future<QJsonObject> pending_;
    QList<QJsonObject> commands_;
    QJsonObject pendingCommand_;
    bool enabled_=true,preferLocal_=false,restoring_=false;
    bool active_=false,ready_=false,dirty_=false,clockDirty_=false;
    qint64 changedAt_=0,nextPoll_=0;
};
QVector<ProfileCandidate> serviceInventory(const QString& host);
// Conflict scan and stop through the installed service: LocalSystem can stop services and elevated
// processes the unelevated editor cannot (owner report 2026-11-16: LEDKeeper2 needed Task Manager).
class ServiceConflictProcesses final : public IConflictProcesses {
public:
    // Runs the service host elevated with these arguments and returns its exit code, or an error
    // (UAC declined, launch failed). Tests replace it; the default shows the UAC prompt.
    using Elevate=std::function<QString(const QStringList& arguments,quint32& exitCode)>;
    explicit ServiceConflictProcesses(ServicePanel::Rpc rpc = {},Elevate elevate = {});
    QVector<ConflictProcess> scan() override;
    QString stop(const ConflictProcess& process, bool force) override;
    QString stopElevated(const ConflictProcess& process) override;
    bool stopsServices() const override { return true; }
private:
    ServicePanel::Rpc rpc_;
    Elevate elevate_;
};
// Message for an exit code of `lighting_service_host.exe --stop-process`; empty for success.
QString stopElevatedExitError(quint32 exitCode);
