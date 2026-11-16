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
    stopConnections();document_=std::move(candidate);project_=bundle.value("project").toObject();
    enabled_=bundle.value("enabled").toBool();playing_=bundle.value("playing").toBool();offset_=time;clock_.restart();paused_=paused;error_.clear();
}
void ServiceRuntime::persist() {
    if(project_.isEmpty())return;
    QJsonArray paused;for(const auto& id:paused_)paused.append(id);
    writeFile(directory_+"/playback.json",QJsonDocument(QJsonObject{{"project",project_},{"enabled",enabled_},{"playing",playing_},{"seconds",seconds()},{"paused",paused}}).toJson(QJsonDocument::Compact));
}
void ServiceRuntime::stopConnections() {
    // Scheduler teardown joins its owner thread before a replacement can open the same endpoint.
    for(auto& connection:connections_)connection->stop();connections_.clear();scopes_.clear();generations_.clear();
}
void ServiceRuntime::suspend(bool value) {
    if(suspended_==value)return;
    offset_=seconds();suspended_=value;clock_.restart();stopConnections();
    if(!value)reconnect();
}
ServiceRuntime::~ServiceRuntime() {
    // The worker posts its result to this object; wait for it before members disappear.
    if(scanner_.joinable())scanner_.join();
}
// Runs on the scan worker when background scanning is on: touches only its arguments.
ServiceRuntime::ScanResult ServiceRuntime::runScan(const Scan& scan,const Guard& guard,const QStringList& targets) {
    ScanResult result;
    try {result.inventory=scan();}
    catch(const std::exception& e) {result.scanFailed=true;result.error=QString::fromUtf8(e.what());return result;}
    if(guard)try {result.blocked=guard(targets,result.error);}
    catch(const std::exception& e) {result.scanFailed=true;result.error=QString::fromUtf8(e.what());}
    if(result.blocked.isEmpty() && !result.scanFailed)result.error.clear();
    return result;
}
QStringList ServiceRuntime::probeTargets() const {
    QStringList result;
    for(const auto& frame:outputScope(*document_)) {
        // Any state counts as held: a Starting worker may be opening the device right now.
        const bool held=std::any_of(scopes_.cbegin(),scopes_.cend(),[&](const auto& scope){return scope.endpoint==frame.endpoint;});
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
    if(scanner_.joinable())scanner_.join();   // The previous worker has already posted its result.
    scanning_=true;waiting_=std::move(next_);next_.clear();
    // The target snapshot is taken here: the worker must not read scopes_.
    scanner_=std::thread([this,scan=scan_,guard=simulation_?Guard{}:guard_,targets=probeTargets()] {
        auto result=runScan(scan,guard,targets);
        QMetaObject::invokeMethod(this,[this,result=std::move(result)] {
            scanning_=false;applyScan(result);
            const auto done=std::move(waiting_);waiting_.clear();
            for(const auto& answer:done)answer();
            if(next_.empty())return;
            if(!suspended_){startScan();return;}
            const auto skipped=std::move(next_);next_.clear();
            for(const auto& answer:skipped)answer();
        },Qt::QueuedConnection);
    });
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
        for(int i=int(connections_.size())-1;i>=0;--i) {
            const auto state=connections_[i]->status().state;
            const bool wanted=std::any_of(frames.cbegin(),frames.cend(),[&](const auto& frame){return sameOutputScope(scopes_[i],frame);});
            if(wanted && (state==OutputScheduler::State::Starting || state==OutputScheduler::State::Running))continue;
            connections_[i]->stop();connections_.erase(connections_.begin()+i);
            scopes_.removeAt(i);generations_.removeAt(i);
        }
        QVector<OutputFrame> missing;
        for(const auto& frame:frames)if(!std::any_of(scopes_.cbegin(),scopes_.cend(),[&](const auto& scope){return sameOutputScope(scope,frame);}))missing.append(frame);
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
        if(op=="project") {loadBundle(request,true);rescan=true;}
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
        } else if(op!="status" && op!="snapshot")throw std::invalid_argument("Unknown service command");
        const auto answer=[this,op,reply]{auto result=status();if(op=="snapshot" && !project_.isEmpty())result["project"]=project_;reply(result);};
        if(rescan)reconnect(answer);else answer();
    } catch(const std::exception& e) {reply({{"ok",false},{"error",QString::fromUtf8(e.what())}});}
}
