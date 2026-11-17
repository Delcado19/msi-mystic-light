#pragma once
#include "output/HardwareOutput.h"
#include <QElapsedTimer>
#include <QJsonObject>
#include <QTimer>
#include <atomic>
#include <mutex>
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
    // Configured endpoints this service does not hold (a hung retired worker counts as holding).
    // Used by the periodic guard; the editor's conflict dialog checks the whole inventory instead.
    QStringList probeTargets() const;
    QVector<ProfileCandidate> inventory() const { return inventory_; }
    // Reopens (and so re-initializes) every output shortly after another program was stopped; calls
    // within 1.5 s collapse into one. Owner report 2026-11-17: after "Terminate all" stopped SignalRGB,
    // the Roccat Vulcan flickered until a project change reopened it (cause inferred, not proven).
    void reopenOutputs() { reopen_.start(); }
private:
    void loadBundle(const QJsonObject& bundle,bool persist);
    void persist();
    void tick();
    // probed: endpoints the guard checked in this scan; with a guard only those may be newly opened.
    struct ScanResult { QVector<ProfileCandidate> inventory; QString error; QStringList blocked,probed; bool scanFailed=false,guarded=false; };
    static ScanResult runScan(const Scan& scan,const Guard& guard,const QStringList& targets);
    void reconnect(std::function<void()> done={});
    void startScan();
    void finishScan(const ScanResult& result);
    // Shared with the scan worker so it never needs `this`; owner is cleared by the destructor.
    struct ScanMailbox { std::mutex mutex; ServiceRuntime* owner=nullptr; std::atomic_bool finished{false}; };
    void applyScan(const ScanResult& result);
    void stopConnections();
    // A worker can hang inside a driver call without timeout (dump 2026-11-17: hidapi's
    // HidD_GetSerialNumberString on the MSI controller 0DB0:0076 never returned, and the main thread
    // joined it forever, so the pipe stopped answering). Stopped workers are therefore never joined on
    // this thread while alive: they wait in retired_ and keep their endpoint closed until they end.
    void retire(int index);
    void reap();
    bool retiring(const QString& endpoint) const;
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
    struct Retired { std::unique_ptr<OutputScheduler> scheduler; QString endpoint; };
    std::vector<Retired> retired_;
    QSet<QString> paused_;
    QJsonObject project_;
    QTimer render_,retry_,reopen_;
    QElapsedTimer clock_;
    QElapsedTimer identifyClock_; // Identify flash expires in the service itself, even if the editor closes.
    double offset_=0;
    bool enabled_=true,playing_=true,suspended_=false,simulation_=false;
    // Background scan: one worker at a time. waiting_ is answered by the running scan, next_ by the
    // following one, so a reply never reports a scan that started before its command.
    std::thread scanner_;
    std::shared_ptr<ScanMailbox> mailbox_=std::make_shared<ScanMailbox>();
    std::vector<std::function<void()>> waiting_,next_;
    bool background_=false,scanning_=false;
};
