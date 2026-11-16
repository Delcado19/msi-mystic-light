#pragma once
#include "output/HardwareOutput.h"
#include <QElapsedTimer>
#include <QJsonObject>
#include <QTimer>
#include <thread>
// Owns the persisted playback session. No client file paths or plugin paths enter this API.
class ServiceRuntime : public QObject {
public:
    using Scan=std::function<QVector<ProfileCandidate>()>;
    using Connect=std::function<QVector<OutputConnection>(const QVector<OutputFrame>&)>;
    ServiceRuntime(ComponentCatalog catalog,QVector<DeviceProfile> profiles,QString directory,Scan scan,Connect connect,bool simulation=false);
    using Reply=std::function<void(const QJsonObject&)>;
    ~ServiceRuntime() override;
    // Synchronous form; only valid while background scanning is off (tests, inline use).
    QJsonObject command(const QJsonObject& command);
    // Replies once the command took effect; commands that rescan hardware reply after that scan.
    void command(const QJsonObject& command,const Reply& reply);
    // Run hardware discovery and the conflict guard on a worker so the caller's thread (the service
    // pipe) keeps answering; a scan took ~6.5 s on the owner's system (measured 2026-11-16).
    void setBackgroundScan(bool value) { background_=value; }
    QJsonObject status() const;
    void suspend(bool suspended);
    // Returns the endpoints another program holds and explains them in message. Only those endpoints
    // stay closed; the others connect (device-based detection, owner decision 2026-11-16).
    using Guard=std::function<QStringList(const QStringList& endpoints,QString& message)>;
    void setOutputGuard(Guard guard) { guard_=std::move(guard); }
    // Configured endpoints this service does not hold. Probing a held endpoint would only find ourselves.
    QStringList probeTargets() const;
    QVector<ProfileCandidate> inventory() const { return inventory_; }
private:
    void loadBundle(const QJsonObject& bundle,bool persist);
    void persist();
    void tick();
    struct ScanResult { QVector<ProfileCandidate> inventory; QString error; QStringList blocked; bool scanFailed=false; };
    static ScanResult runScan(const Scan& scan,const Guard& guard,const QStringList& targets);
    void reconnect(std::function<void()> done={});
    void startScan();
    void applyScan(const ScanResult& result);
    void stopConnections();
    double seconds() const;
    QString directory_, error_;
    ComponentCatalog catalog_;
    QVector<DeviceProfile> profiles_;
    std::unique_ptr<LightingDocument> document_;
    Scan scan_;
    Connect connect_;
    Guard guard_;
    QVector<ProfileCandidate> inventory_;
    std::vector<std::unique_ptr<OutputScheduler>> connections_;
    QVector<OutputFrame> scopes_;
    QVector<quint64> generations_;
    QSet<QString> paused_;
    QJsonObject project_;
    QTimer render_,retry_;
    QElapsedTimer clock_;
    QElapsedTimer identifyClock_; // Identify flash expires in the service itself, even if the editor closes.
    double offset_=0;
    bool enabled_=true,playing_=true,suspended_=false,simulation_=false;
    // Background scan: one worker at a time. waiting_ is answered by the running scan, next_ by the
    // following one, so a reply never reports a scan that started before its command.
    std::thread scanner_;
    std::vector<std::function<void()>> waiting_,next_;
    bool background_=false,scanning_=false;
};
