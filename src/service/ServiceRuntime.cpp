#include "ServiceRuntime.h"
#include "ServiceProtocol.h"
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <QTemporaryFile>
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace {
QString key(const OutputFrame& frame,const ChannelOutput& channel) { return frame.endpoint+"\n"+frame.profileId+"\n"+channel.channelId; }
void writeFile(const QString& path,const QByteArray& bytes) {
    QSaveFile file(path);if(!file.open(QIODevice::WriteOnly) || file.write(bytes)!=bytes.size() || !file.commit())
        throw std::runtime_error("Cannot persist service project");
}
}
ServiceRuntime::ServiceRuntime(ComponentCatalog catalog,QVector<DeviceProfile> profiles,QString directory,Scan scan,Connect connect,bool simulation)
    : directory_(std::move(directory)),catalog_(std::move(catalog)),profiles_(std::move(profiles)),scan_(std::move(scan)),connect_(std::move(connect)),simulation_(simulation)
{
    if(!QDir().mkpath(directory_))throw std::runtime_error("Cannot open service state directory");
    document_=std::make_unique<LightingDocument>(catalog_,nullptr,profiles_);clock_.start();
    QFile saved(directory_+"/playback.json");
    if(saved.exists())try {
        if(!saved.open(QIODevice::ReadOnly) || saved.size()>StudioService::maximumMessage)throw std::runtime_error("Cannot read saved service project");
        loadBundle(StudioService::decode(saved.readAll()),false);
    } catch(const std::exception& e) {error_=QString("Saved project rejected: ")+e.what();enabled_=false;}
    render_.setInterval(33);render_.setTimerType(Qt::PreciseTimer);QObject::connect(&render_,&QTimer::timeout,this,[this]{tick();});render_.start();
    retry_.setInterval(5000);QObject::connect(&retry_,&QTimer::timeout,this,[this]{reconnect();});retry_.start();
    // A normal scan takes ~6.5 s. 12 s stays below the editor's 15 s request timeout, so a caller
    // receives the "did not finish" reply instead of timing out (owner decision 2026-11-17).
    scanDeadline_.setSingleShot(true);scanDeadline_.setInterval(12000);
    QObject::connect(&scanDeadline_,&QTimer::timeout,this,[this]{abandonScan();});
    reopen_.setSingleShot(true);reopen_.setInterval(1500);
    QObject::connect(&reopen_,&QTimer::timeout,this,[this]{if(suspended_ || !enabled_)return;stopConnections();reconnect();});
    QTimer::singleShot(0,this,[this]{reconnect();});
}
double ServiceRuntime::seconds() const {
    const auto time=offset_+(playing_ && !suspended_ ? clock_.elapsed()/1000.0:0);
    // Keep the document's bounded clock while allowing a service to animate beyond one day.
    return playing_ && !suspended_ ? std::fmod(time,86400.0):time;
}
void ServiceRuntime::loadBundle(const QJsonObject& bundle,bool save) {
    if(!bundle.value("project").isObject() || !bundle.value("enabled").isBool() || !bundle.value("playing").isBool() || !bundle.value("seconds").isDouble())
        throw std::invalid_argument("Project, enabled, playing and seconds are required");
    const double time=bundle.value("seconds").toDouble();if(!std::isfinite(time)||time<0||time>86400)throw std::invalid_argument("Invalid playback time");
    QTemporaryFile file(directory_+"/validate-XXXXXX.json");if(!file.open())throw std::runtime_error("Cannot stage service project");
    const auto bytes=QJsonDocument(bundle.value("project").toObject()).toJson(QJsonDocument::Compact);
    if(bytes.size()>StudioService::maximumMessage || file.write(bytes)!=bytes.size() || !file.flush())throw std::runtime_error("Cannot stage bounded project");
    auto candidate=std::make_unique<LightingDocument>(catalog_,nullptr,profiles_);candidate->load(file.fileName());
    // Validate the complete project before replacing either the running or persisted session.
    const auto frames=prepareOutput(*candidate,candidate->render(time),time);
    QSet<QString> valid;for(const auto& frame:frames)for(const auto& channel:frame.channels)valid.insert(key(frame,channel));
    auto paused=paused_;paused.intersect(valid);
    if(bundle.contains("paused")) {
        if(!bundle.value("paused").isArray())throw std::invalid_argument("Invalid paused channels");
        paused.clear();for(const auto& entry:bundle.value("paused").toArray()) {
            if(!entry.isString() || !valid.contains(entry.toString()))throw std::invalid_argument("Unknown paused channel");
            paused.insert(entry.toString());
        }
    }
    auto saved=bundle;QJsonArray pausedJson;for(const auto& entry:paused)pausedJson.append(entry);saved["paused"]=pausedJson;
    if(save)writeFile(directory_+"/playback.json",StudioService::encode(saved).mid(4));
    // Running outputs whose scope is unchanged stay open: tick() matches frames to connections by
    // output scope. Closing all of them froze the lights for a whole scan on every drag (owner report
    // 2026-11-17). Changed scopes, and everything when output is disabled, are released right here.
    const auto keep=bundle.value("enabled").toBool() ? frames : QVector<OutputFrame>{}; // Same shape as scopes_.
    retireAndWait([&](int i){return std::none_of(keep.cbegin(),keep.cend(),[&](const OutputFrame& frame){return sameOutputScope(scopes_[i],frame);});});
    document_=std::move(candidate);project_=bundle.value("project").toObject();
    enabled_=bundle.value("enabled").toBool();playing_=bundle.value("playing").toBool();offset_=time;clock_.restart();paused_=paused;error_.clear();
}
void ServiceRuntime::persist() {
    if(project_.isEmpty())return;
    QJsonArray paused;for(const auto& id:paused_)paused.append(id);
    writeFile(directory_+"/playback.json",QJsonDocument(QJsonObject{{"project",project_},{"enabled",enabled_},{"playing",playing_},{"seconds",seconds()},{"paused",paused}}).toJson(QJsonDocument::Compact));
}
void ServiceRuntime::retire(int index) {
    connections_[index]->stop();
    retired_.push_back({std::move(connections_[index]),scopes_[index].endpoint});
    connections_.erase(connections_.begin()+index);scopes_.removeAt(index);generations_.removeAt(index);
}
void ServiceRuntime::reap() {
    // Only finished workers are destroyed here; their join returns at once.
    retired_.erase(std::remove_if(retired_.begin(),retired_.end(),[](const Retired& retired) {
        const auto state=retired.scheduler->status().state;
        return state==OutputScheduler::State::Stopped || state==OutputScheduler::State::Faulted;
    }),retired_.end());
}
bool ServiceRuntime::retiring(const QString& endpoint) const {
    return std::any_of(retired_.cbegin(),retired_.cend(),[&](const Retired& retired){return retired.endpoint==endpoint;});
}
void ServiceRuntime::stopConnections() {
    retireAndWait([](int){return true;});
}
void ServiceRuntime::retireAndWait(const std::function<bool(int)>& unwanted) {
    std::vector<const OutputScheduler*> stopped;
    for(int i=int(connections_.size())-1;i>=0;--i)if(unwanted(i)) {stopped.push_back(connections_[i].get());retire(i);}
    // A healthy worker ends within one HID write; give the ones stopped here one bounded second so the
    // same scan (or an immediate reconnect) can reopen their endpoints. Without the wait a retired
    // worker was still Stopping at reap(), its endpoint counted as held and reopened two scans late
    // (final review 2026-11-17). Workers that hung in an earlier call are not waited for again, or
    // every project sync would freeze rendering for 1 s.
    const auto pending=[&] {return std::any_of(retired_.cbegin(),retired_.cend(),[&](const Retired& retired) {
        return std::find(stopped.cbegin(),stopped.cend(),retired.scheduler.get())!=stopped.cend();});};
    QElapsedTimer waited;waited.start();
    for(reap();pending() && waited.elapsed()<1000;reap())std::this_thread::sleep_for(std::chrono::milliseconds(5));
}
void ServiceRuntime::suspend(bool value) {
    if(suspended_==value)return;
    offset_=seconds();suspended_=value;clock_.restart();stopConnections();
    if(!value)reconnect();
}
ServiceRuntime::~ServiceRuntime() {
    stopConnections();
    // shortcut: a worker still hung in a driver call is leaked instead of joined, so the service can
    // stop (sc stop hung in STOP_PENDING, 2026-11-17); safe only because the process exits next.
    for(auto& retired:retired_)(void)retired.scheduler.release();
    // hid_enumerate reads serial numbers too, so a scan can hang like an open. Wait up to 10 s (a
    // normal scan takes ~6.5 s); a scanner still running then is abandoned. It posts its result only
    // through the shared mailbox, which no longer names this object (CodeRabbit review 2026-11-17).
    // A result posted but not yet delivered is dropped by ~QObject.
    if(scanner_.joinable()) {
        QElapsedTimer waited;waited.start();
        while(!mailbox_->finished.load() && waited.elapsed()<10000)std::this_thread::sleep_for(std::chrono::milliseconds(10));
        {const std::lock_guard<std::mutex> lock(mailbox_->mutex);mailbox_->owner=nullptr;}
        if(mailbox_->finished.load())scanner_.join();else scanner_.detach();
    }
}
// Runs on the scan worker when background scanning is on: touches only its arguments.
ServiceRuntime::ScanResult ServiceRuntime::runScan(const Scan& scan,const Guard& guard,const QStringList& targets) {
    ScanResult result;
    try {result.inventory=scan();}
    catch(const std::exception& e) {result.scanFailed=true;result.error=QString::fromUtf8(e.what());return result;}
    // Probe only endpoints this scan discovered as supported lighting devices, never a bare project path:
    // a holder counts as proven for the LocalSystem stop path (security review 2026-11-16).
    QStringList discovered;
    for(const auto& target:targets)if(std::any_of(result.inventory.cbegin(),result.inventory.cend(),
        [&](const ProfileCandidate& c){return c.device.path.compare(target,Qt::CaseInsensitive)==0;}))discovered.append(target);
    result.probed=discovered;result.guarded=bool(guard);
    if(guard)try {result.blocked=guard(discovered,result.error);}
    catch(const std::exception& e) {result.scanFailed=true;result.error=QString::fromUtf8(e.what());}
    return result;
}
QStringList ServiceRuntime::probeTargets() const {
    QStringList result;
    for(const auto& frame:outputScope(*document_)) {
        // A Starting worker may be opening the device right now, and a retired one may still hold it.
        // A Faulted or Stopped one holds nothing and is probed before it is reopened (CodeRabbit 2026-11-16).
        bool held=retiring(frame.endpoint);
        for(int i=0;i<scopes_.size() && !held;++i)if(scopes_[i].endpoint==frame.endpoint) {
            const auto state=connections_[i]->status().state;
            held=state==OutputScheduler::State::Starting || state==OutputScheduler::State::Running;
        }
        if(!held && !result.contains(frame.endpoint))result.append(frame.endpoint);
    }
    return result;
}
void ServiceRuntime::reconnect(std::function<void()> done) {
    if(suspended_) {if(done)done();return;}
    if(!background_) {applyScan(runScan(scan_,simulation_?Guard{}:guard_,probeTargets()));if(done)done();return;}
    // A periodic retry (no done) adds nothing to a scan that is already running; a command needs a fresh one.
    if(scanning_ && !done)return;
    if(!done)done=[]{};
    next_.push_back(std::move(done));
    if(!scanning_)startScan();
}
void ServiceRuntime::startScan() {
    // A scan abandoned at its deadline may still hang in the driver. Start no second one meanwhile, or
    // every retry would add another stuck thread; answer the callers with the error instead.
    if(abandoned_ && !abandoned_->finished.load()) {
        error_=scanStuckError(); // loadBundle cleared it; the reply must still say discovery is blocked.
        const auto skipped=std::move(next_);next_.clear();
        for(const auto& answer:skipped)answer();
        return;
    }
    abandoned_.reset();
    if(scanner_.joinable())scanner_.join();   // The previous worker has already posted its result.
    scanning_=true;waiting_=std::move(next_);next_.clear();
    scanDeadline_.start();
    // The target snapshot is taken here: the worker must not read scopes_. It never touches `this`;
    // the mailbox is cleared by the destructor before an abandoned worker could use it.
    mailbox_=std::make_shared<ScanMailbox>();mailbox_->owner=this;
    scanner_=std::thread([scan=scan_,guard=simulation_?Guard{}:guard_,targets=probeTargets(),mailbox=mailbox_] {
        struct Done {std::shared_ptr<ScanMailbox> box;~Done(){box->finished.store(true);}} done{mailbox};
        auto result=runScan(scan,guard,targets);
        const std::lock_guard<std::mutex> lock(mailbox->mutex);
        if(auto* owner=mailbox->owner)
            // The result may already be queued when the deadline abandons this scan; clearing owner
            // does not cancel a posted call, so a superseded mailbox is rejected here (CodeRabbit 2026-11-17).
            QMetaObject::invokeMethod(owner,[owner,mailbox,result=std::move(result)] {
                if(owner->mailbox_==mailbox && owner->scanning_)owner->finishScan(result);
            },Qt::QueuedConnection);
    });
}
void ServiceRuntime::abandonScan() {
    if(!scanning_)return;
    // hid_enumerate reads serial numbers and can hang like a device open (CodeRabbit review 2026-11-17:
    // a hung scan blocked retries and replies until restart). Its late result is never delivered.
    {const std::lock_guard<std::mutex> lock(mailbox_->mutex);mailbox_->owner=nullptr;}
    abandoned_=mailbox_;scanner_.detach();scanning_=false;
    error_=scanStuckError();
    const auto done=std::move(waiting_);waiting_.clear();
    for(const auto& answer:done)answer();
    if(!next_.empty())startScan();
}
void ServiceRuntime::finishScan(const ScanResult& result) {
    scanDeadline_.stop();
    scanning_=false;applyScan(result);
    const auto done=std::move(waiting_);waiting_.clear();
    for(const auto& answer:done)answer();
    if(next_.empty())return;
    if(!suspended_){startScan();return;}
    const auto skipped=std::move(next_);next_.clear();
    for(const auto& answer:skipped)answer();
}
// connect_ opens hardware and must not overlap a scan; it only runs here, between two scans.
void ServiceRuntime::applyScan(const ScanResult& result) {
    if(suspended_)return;
    try {
        if(result.scanFailed)throw std::runtime_error(result.error.toStdString());
        inventory_=result.inventory;
        if(simulation_) {error_.clear();return;}
        if(!enabled_)return;
        // Blocked endpoints stay closed and are reported; every other device keeps or gets its output.
        error_=result.error;
        auto frames=prepareOutput(*document_,document_->render(seconds()),seconds());
        frames.removeIf([&](const OutputFrame& frame){return result.blocked.contains(frame.endpoint);});
        // Offline saved endpoints are omitted by connect_; retry them without resetting healthy hardware.
        retireAndWait([&](int i) {
            const auto state=connections_[i]->status().state;
            const bool wanted=std::any_of(frames.cbegin(),frames.cend(),[&](const auto& frame){return sameOutputScope(scopes_[i],frame);});
            return !wanted || (state!=OutputScheduler::State::Starting && state!=OutputScheduler::State::Running);
        });
        // An endpoint whose old worker is still alive is not reopened: a hung driver call would only
        // gain a second stuck worker on every retry.
        QVector<OutputFrame> missing;
        // An endpoint is opened only if this scan probed it for other writers; one added or freed while
        // the scan ran waits for the next scan (CodeRabbit review 2026-11-16).
        for(const auto& frame:frames)if(!retiring(frame.endpoint) && (!result.guarded || result.probed.contains(frame.endpoint)) &&
            !std::any_of(scopes_.cbegin(),scopes_.cend(),[&](const auto& scope){return sameOutputScope(scope,frame);}))missing.append(frame);
        if(missing.isEmpty())return;
        const auto prepared=connect_(missing);
        for(const auto& connection:prepared) {
            auto scheduler=std::make_unique<OutputScheduler>();
            const auto generation=scheduler->start(connection.factory,connection.present);
            if(!generation)throw std::runtime_error("Cannot start output worker");
            scopes_.append(connection.initial);generations_.append(generation);connections_.push_back(std::move(scheduler));
        }
    } catch(const std::exception& e) {error_=QString::fromUtf8(e.what());stopConnections();}
}
void ServiceRuntime::tick() {
    if(!document_->identifyPlacement().isNull() && identifyClock_.elapsed()>=2000)document_->setIdentifyPlacement({});
    // Rendering 30 frames per second for no connected device only costs CPU (audit 2026-11-14).
    if(!enabled_ || suspended_ || connections_.empty())return;
    try {
        const auto time=seconds();auto frames=prepareOutput(*document_,document_->render(time),time);
        for(int i=0;i<int(connections_.size());++i)for(auto frame:frames)if(sameOutputScope(scopes_[i],frame)) {
            for(auto it=frame.channels.begin();it!=frame.channels.end();)if(paused_.contains(key(frame,*it)))it=frame.channels.erase(it);else ++it;
            const auto state=connections_[i]->status().state;
            if(state==OutputScheduler::State::Starting || state==OutputScheduler::State::Running)connections_[i]->submit(generations_[i],std::move(frame));
        }
    } catch(const std::exception& e) {error_=QString::fromUtf8(e.what());stopConnections();}
}
QJsonObject ServiceRuntime::status() const {
    QJsonArray devices,outputs;
    for(const auto& item:inventory_)devices.append(QJsonObject{{"profileId",item.profileId},{"path",item.device.path},{"transport",item.device.transport},{"vendorId",item.device.vendorId},{"productId",item.device.productId},{"interfaceNumber",item.device.interfaceNumber},{"usagePage",item.device.usagePage},{"usage",item.device.usage}});
    // Status lists channels only; outputScope neither renders nor samples.
    const auto frames=outputScope(*document_);
    for(const auto& frame:frames) {
        QString state=enabled_?"Offline":"Stopped",error;
        if(retiring(frame.endpoint)) {state="Not responding";error="The device did not answer a driver call; it is retried once that call returns";}
        for(int i=0;i<scopes_.size();++i)if(scopes_[i].endpoint==frame.endpoint) {
            const auto s=connections_[i]->status();error=s.error;
            state=s.state==OutputScheduler::State::Running?"Running":s.state==OutputScheduler::State::Starting?"Connecting":s.state==OutputScheduler::State::Faulted?"Faulted":"Stopped";
        }
        for(const auto& channel:frame.channels)outputs.append(QJsonObject{{"endpoint",frame.endpoint},{"profileId",frame.profileId},{"channel",channel.channelId},{"key",key(frame,channel)},{"state",paused_.contains(key(frame,channel))?"Paused":state},{"error",error}});
    }
    return {{"ok",true},{"version",StudioService::protocolVersion},{"simulation",simulation_},{"enabled",enabled_},{"playing",playing_},{"seconds",seconds()},{"devices",devices},{"outputs",outputs},{"error",error_}};
}
QJsonObject ServiceRuntime::command(const QJsonObject& request) {
    QJsonObject result;command(request,[&result](const QJsonObject& reply){result=reply;});return result;
}
void ServiceRuntime::command(const QJsonObject& request,const Reply& reply) {
    try {
        bool rescan=false;
        if(request.value("version").toInt()!=StudioService::protocolVersion)throw std::invalid_argument("Unsupported service protocol");
        const auto op=request.value("op").toString();
        // The project is saved and loaded at once; its reply need not wait for the hardware scan,
        // which could exceed the editor's 15 s deadline and trigger a needless resend (CodeRabbit
        // review 2026-11-16). A fresh scan still runs; the editor's status poll shows its result.
        if(op=="project") {loadBundle(request,true);reconnect([]{});}
        else if(op=="scan")rescan=true;
        else if(op=="playback") {
            if(!request.value("playing").isBool() || !request.value("seconds").isDouble())throw std::invalid_argument("Invalid playback command");
            const double time=request.value("seconds").toDouble();if(!std::isfinite(time)||time<0||time>86400)throw std::invalid_argument("Invalid playback time");
            const auto oldTime=seconds();const bool oldPlaying=playing_;
            offset_=time;playing_=request.value("playing").toBool();clock_.restart();
            try{persist();}catch(...){offset_=oldTime;playing_=oldPlaying;clock_.restart();throw;}
        } else if(op=="output") {
            if(!request.value("enabled").isBool())throw std::invalid_argument("Invalid output command");
            const bool old=enabled_;enabled_=request.value("enabled").toBool();
            try{persist();}catch(...){enabled_=old;throw;}
            if(enabled_)rescan=true;else stopConnections();
        } else if(op=="channels") {
            if(!request.value("keys").isArray() || !request.value("paused").isBool())throw std::invalid_argument("Invalid channel command");
            QSet<QString> valid;for(const auto& f:outputScope(*document_))for(const auto& c:f.channels)valid.insert(key(f,c));
            auto next=paused_;for(const auto& id:request.value("keys").toArray()) {
                if(!id.isString() || !valid.contains(id.toString()))throw std::invalid_argument("Unknown channel");
                if(request.value("paused").toBool())next.insert(id.toString());else next.remove(id.toString());
            }
            const auto before=paused_;paused_=next;try{persist();}catch(...){paused_=before;throw;}
        } else if(op=="identify") {
            // Transient 2 s flash of one software component; never persisted or part of the snapshot.
            const QUuid placement(request.value("placement").toString());
            bool known=false;for(const auto& item:document_->state().placements)if(item.id==placement)known=true;
            if(placement.isNull() || !known || !document_->hasSoftwareChannel(placement))throw std::invalid_argument("Unknown identify component");
            document_->setIdentifyPlacement(placement);identifyClock_.start();
        } else if(op=="reopen")reopenOutputs(); // Sent by the editor after an administrator (UAC) stop.
        else if(op!="status" && op!="snapshot")throw std::invalid_argument("Unknown service command");
        const auto answer=[this,op,reply]{auto result=status();if(op=="snapshot" && !project_.isEmpty())result["project"]=project_;reply(result);};
        if(rescan)reconnect(answer);else answer();
    } catch(const std::exception& e) {reply({{"ok",false},{"error",QString::fromUtf8(e.what())}});}
}
