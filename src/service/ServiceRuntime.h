#pragma once
#include "output/HardwareOutput.h"
#include <QElapsedTimer>
#include <QJsonObject>
#include <QTimer>
// Owns the persisted playback session. No client file paths or plugin paths enter this API.
class ServiceRuntime : public QObject {
public:
    using Scan=std::function<QVector<ProfileCandidate>()>;
    using Connect=std::function<QVector<OutputConnection>(const QVector<OutputFrame>&)>;
    ServiceRuntime(ComponentCatalog catalog,QVector<DeviceProfile> profiles,QString directory,Scan scan,Connect connect,bool simulation=false);
    QJsonObject command(const QJsonObject& command);
    QJsonObject status() const;
    void suspend(bool suspended);
    void setOutputGuard(std::function<void()> guard) { guard_=std::move(guard); }
private:
    void loadBundle(const QJsonObject& bundle,bool persist);
    void persist();
    void tick();
    void reconnect();
    void stopConnections();
    double seconds() const;
    QString directory_, error_;
    ComponentCatalog catalog_;
    QVector<DeviceProfile> profiles_;
    std::unique_ptr<LightingDocument> document_;
    Scan scan_;
    Connect connect_;
    std::function<void()> guard_;
    QVector<ProfileCandidate> inventory_;
    std::vector<std::unique_ptr<OutputScheduler>> connections_;
    QVector<OutputFrame> scopes_;
    QVector<quint64> generations_;
    QSet<QString> paused_;
    QJsonObject project_;
    QTimer render_,retry_;
    QElapsedTimer clock_;
    double offset_=0;
    bool enabled_=true,playing_=true,suspended_=false,simulation_=false;
};
