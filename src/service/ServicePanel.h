#pragma once
#include "editor/LightingWindow.h"
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
    void send(QJsonObject command);
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
