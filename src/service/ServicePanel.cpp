#include "ServicePanel.h"
#include "ServiceProtocol.h"
#include "startup/ConflictDialog.h"
#include "editor/LightingCanvas.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDoubleSpinBox>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QPushButton>
#include <QStatusBar>
#include <QMessageBox>
#include <QHeaderView>
#include <QSaveFile>
#include <QScopedValueRollback>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <stdexcept>
#ifdef Q_OS_WIN
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#endif
namespace {
QString host() { const auto installed=StudioService::installedHost();return installed.isEmpty()?QCoreApplication::applicationDirPath()+"/lighting_service_host.exe":installed; }
QJsonObject command(const QString& op) {return {{"op",op},{"version",StudioService::protocolVersion}};}
}
QVector<ProfileCandidate> serviceInventory(const QString& executable) {
    const auto reply=StudioService::request(command("scan"),executable);QVector<ProfileCandidate> result;
    for(const auto& entry:reply.value("devices").toArray()) {
        const auto json=entry.toObject();DeviceIdentity d;d.path=json.value("path").toString();d.transport=json.value("transport").toString();
        d.vendorId=quint16(json.value("vendorId").toInt());d.productId=quint16(json.value("productId").toInt());
        d.interfaceNumber=json.value("interfaceNumber").toInt();d.usagePage=quint16(json.value("usagePage").toInt());d.usage=quint16(json.value("usage").toInt());
        result.append({d,json.value("profileId").toString()});
    }
    return result;
}
ServiceConflictProcesses::ServiceConflictProcesses(ServicePanel::Rpc rpc,Elevate elevate):rpc_(std::move(rpc)),elevate_(std::move(elevate)) {
    if(!rpc_)rpc_=[](const QJsonObject& request){return StudioService::request(request,host());};
    if(!elevate_)elevate_=[](const QStringList& arguments,quint32& exitCode)->QString {
#ifdef Q_OS_WIN
        // The protected Program Files host, never a path from the service reply or the project.
        const auto executable=QDir::toNativeSeparators(host());const auto parameters=arguments.join(' ');
        SHELLEXECUTEINFOW launch{};launch.cbSize=sizeof(launch);launch.fMask=SEE_MASK_NOCLOSEPROCESS|SEE_MASK_FLAG_NO_UI;
        if(auto* owner=QApplication::activeModalWidget())launch.hwnd=reinterpret_cast<HWND>(owner->winId());
        launch.lpVerb=L"runas";launch.lpFile=reinterpret_cast<LPCWSTR>(executable.utf16());
        launch.lpParameters=reinterpret_cast<LPCWSTR>(parameters.utf16());launch.nShow=SW_HIDE;
        if(!ShellExecuteExW(&launch)) {
            const auto failure=GetLastError();
            return failure==ERROR_CANCELLED?QString("Administrator approval was declined"):
                "Cannot start the administrator stop (Windows error "+QString::number(failure)+")";
        }
        if(!launch.hProcess)return "Cannot track the administrator stop; rescan";
        // A stop takes milliseconds; a service stop is only requested, not awaited.
        const auto wait=WaitForSingleObject(launch.hProcess,15000);DWORD code=1;
        const bool done=wait==WAIT_OBJECT_0 && GetExitCodeProcess(launch.hProcess,&code);CloseHandle(launch.hProcess);
        if(!done)return "The administrator stop did not finish in 15 s; rescan";
        exitCode=code;return {};
#else
        Q_UNUSED(arguments);Q_UNUSED(exitCode);return "Administrator stop is supported on Windows only";
#endif
    };
}
QString stopElevatedExitError(quint32 exitCode) {
    switch(exitCode) {
    case 0: return {};
    case 3: return "Process identity changed; rescan before trying again";
    case 4: return "Refused: Windows and Lighting Studio processes are never stopped";
    default: return "Administrator stop failed (exit code "+QString::number(exitCode)+")";
    }
}
QString ServiceConflictProcesses::stopElevated(const ConflictProcess& process) {
    if(!process.elevatedStop || !process.pid || !process.started)return "Refusing an invalid administrator stop";
    quint32 code=1;
    const auto error=elevate_({"--stop-process",QString::number(process.pid),QString::number(process.started)},code);
    const auto result=error.isEmpty()?stopElevatedExitError(code):error;
    // The service did not see this stop; ask it to re-initialize its outputs (see reopenOutputs).
    if(result.isEmpty())try{rpc_(command("reopen"));}catch(const std::exception&){}
    return result;
}
QVector<ConflictProcess> ServiceConflictProcesses::scan() {
    QVector<ConflictProcess> result;const auto reply=rpc_(command("conflicts"));
    // A failed USB capture must not look like "nothing conflicts"; the dialog shows the error.
    const auto list=reply.value("conflicts").toArray();
    if(list.isEmpty() && !reply.value("scanError").toString().isEmpty())throw std::runtime_error(reply.value("scanError").toString().toStdString());
    for(const auto& entry:list) {
        const auto json=entry.toObject();ConflictProcess process;
        process.pid=quint32(json.value("pid").toInteger());process.started=json.value("started").toString().toULongLong();
        process.executable=json.value("executable").toString();process.path=json.value("path").toString();
        process.reason=json.value("reason").toString();process.manualStop=json.value("manualStop").toBool();process.elevatedStop=json.value("elevatedStop").toBool();result.append(process);
    }
    return result;
}
QString ServiceConflictProcesses::stop(const ConflictProcess& process,bool force) {
    // The service runs in session 0 and cannot post WM_CLOSE to desktop windows, so the polite close
    // request comes from the editor; its result does not matter, the service step decides.
    if(!force)requestClose(process);
    auto request=command("stopConflict");request["pid"]=qint64(process.pid);request["started"]=QString::number(process.started);request["force"]=force;
    try {return rpc_(request).value("stopError").toString();}
    catch(const std::exception& e) {return QString::fromUtf8(e.what());}
}
ServicePanel::ServicePanel(LightingWindow& window,Rpc rpc):QWidget(&window),rpc_(std::move(rpc)),window_(window) {
    if(!rpc_)rpc_=[](const QJsonObject& request){return StudioService::request(request,host());};
    setObjectName("outputPanel");auto* layout=new QVBoxLayout(this);
    auto* note=new QLabel("Windows service owns hardware output. Edits synchronize automatically; closing the editor keeps lighting running.");
    note->setWordWrap(true);layout->addWidget(note);
    auto* buttons=new QHBoxLayout;layout->addLayout(buttons);
    const auto button=[&](const QString& text,const std::function<void()>& action){auto* b=new QPushButton(text);buttons->addWidget(b);connect(b,&QPushButton::clicked,this,action);};
    button("Resume all output",[this]{auto c=command("output");c["enabled"]=true;send(c);});
    button("Stop all output",[this]{stopOutput();});
    // A conflict can appear while the editor is open (another program starts later); the dialog must be
    // reachable then too, not only when the window opens (owner request 2026-11-16).
    button("Resolve conflicts...",[this]{
        try {
            ServiceConflictProcesses processes(rpc_);
            if(processes.scan().isEmpty()){status_->setText("No other program holds a configured lighting device.");return;}
            ConflictDialog dialog(processes,&window_);dialog.exec();
        } catch(const std::exception& e){status_->setText(QString::fromUtf8(e.what()));return;}
        send(command("scan"));
    });
    for(const bool pause:{false,true})button(pause?"Pause selected":"Resume selected",[this,pause]{
        QJsonArray keys;for(const auto* row:outputs_->selectedItems())keys.append(row->data(0,Qt::UserRole).toString());
        if(keys.isEmpty()){status_->setText("Select one or more output channels first.");return;}
        auto c=command("channels");c["keys"]=keys;c["paused"]=pause;send(c);
    });
    status_=new QLabel;status_->setObjectName("outputStatus");status_->setTextFormat(Qt::PlainText);status_->setWordWrap(true);layout->addWidget(status_);
    outputs_=new QTreeWidget;outputs_->setObjectName("outputEndpoints");outputs_->setHeaderLabels({"Endpoint / channel","Profile","State"});
    outputs_->setSelectionMode(QAbstractItemView::ExtendedSelection);outputs_->setTextElideMode(Qt::ElideMiddle);
    outputs_->header()->setSectionResizeMode(0,QHeaderView::Stretch);
    outputs_->header()->setSectionResizeMode(1,QHeaderView::ResizeToContents);
    outputs_->header()->setSectionResizeMode(2,QHeaderView::ResizeToContents);layout->addWidget(outputs_,1);
    connect(&window_.document(),&LightingDocument::changed,this,[this]{if(active_ && !restoring_)dirty_=true;});
    // A drag changes the document only on release; its working state streams meanwhile (owner review 2026-11-18).
    for(auto* canvas:window_.findChildren<LightingCanvas*>())
        connect(canvas,&LightingCanvas::workingStateChanged,this,[this]{if(active_ && !restoring_)dirty_=true;});
    connect(window_.findChild<QPushButton*>("playPreview"),&QPushButton::toggled,this,[this]{if(ready_)clockDirty_=true;});
    // Clock value changes during normal playback need no IPC; the service advances its own clock.
    connect(window_.findChild<QDoubleSpinBox*>("previewTime"),&QDoubleSpinBox::editingFinished,this,[this]{if(ready_)clockDirty_=true;});
    connect(&window_,&LightingWindow::closing,this,[this]{flushOnClose();});
    // The service owns output, so it must flash too; it expires the flash itself (CodeRabbit review 2026-11-15).
    connect(&window_,&LightingWindow::identifyRequested,this,[this](const QUuid& placement){
        if(!ready_)return;
        // A just-attached component is unknown to the service until the pending project update arrives, so queue it first.
        // Clear the flags only once the queue accepted the project, so poll()/flushOnClose() can retry it (CodeRabbit review 2026-11-15).
        if(dirty_){try{if(!send(project()))return;dirty_=false;clockDirty_=false;}catch(const std::exception& e){status_->setText(QString::fromUtf8(e.what()));return;}}
        auto c=command("identify");c["placement"]=placement.toString();send(c);});
    timer_.setInterval(50);connect(&timer_,&QTimer::timeout,this,[this]{poll();});
}
void ServicePanel::activate(bool preferLocal) {preferLocal_=preferLocal;active_=true;send(command("snapshot"));timer_.start();}
void ServicePanel::stopOutput(){auto c=command("output");c["enabled"]=false;send(c);}
bool ServicePanel::send(QJsonObject c) {
    if(commands_.size()>=16){status_->setText("Service command queue full; wait for the current operation.");return false;}
    commands_.append(c);return true;
}
QJsonObject ServicePanel::project() {
    QTemporaryDir folder;if(!folder.isValid())throw std::runtime_error("Cannot stage editor project");
    std::optional<LightingState> live;
    for(auto* canvas:window_.findChildren<LightingCanvas*>())if(!live)live=canvas->workingState();
    QJsonArray definitions;
    const auto path=folder.path()+"/project.json";window_.document().save(path,false,live ? &*live : nullptr,&definitions);
    QFile input(path);if(!input.open(QIODevice::ReadOnly))throw std::runtime_error("Cannot read staged editor project");
    auto result=command("project");result["project"]=StudioService::decode(input.readAll());result["componentDefinitions"]=definitions;
    // The service applies the snapshot's enabled flag, so an unconfirmed Stop/Resume must win over the
    // last reported state; otherwise a project sent after Stop re-enables output (CodeRabbit review 2026-11-15).
    bool enabled=enabled_;
    if(pending_.valid() && pendingCommand_.value("op")=="output")enabled=pendingCommand_.value("enabled").toBool();
    for(const auto& queued:commands_)if(queued.value("op")=="output")enabled=queued.value("enabled").toBool();
    result["enabled"]=enabled;result["playing"]=window_.findChild<QPushButton*>("playPreview")->isChecked();result["seconds"]=window_.previewSeconds();return result;
}
void ServicePanel::display(const QJsonObject& reply) {
    enabled_=reply.value("enabled").toBool();
    QSet<QString> selected;for(const auto* row:outputs_->selectedItems())selected.insert(row->data(0,Qt::UserRole).toString());
    outputs_->clear();int running=0,errors=0;
    QStringList messages;const auto discovery=reply.value("discoveryError").toString();if(!discovery.isEmpty())messages.append("RAM discovery: "+discovery);const auto error=reply.value("error").toString();if(!error.isEmpty())messages.append(error);
    for(const auto& entry:reply.value("outputs").toArray()) {
        const auto item=entry.toObject();auto* row=new QTreeWidgetItem(outputs_,{item.value("endpoint").toString()+" / "+item.value("channel").toString(),item.value("profileId").toString(),item.value("state").toString()});
        row->setToolTip(0,row->text(0));row->setData(0,Qt::UserRole,item.value("key").toString());row->setSelected(selected.contains(item.value("key").toString()));
        if(item.value("state").toString()=="Running")++running;
        const auto failure=item.value("error").toString();if(!failure.isEmpty()){++errors;messages.append(failure);row->setToolTip(2,failure);}
    }
    messages.removeDuplicates();
    status_->setText(messages.isEmpty()?"Service connected. Playback continues when the editor closes.":messages.join("\n"));
    // A blocked device used to stay dark with the reason only in Devices and a tooltip (owner report
    // 2026-11-26: mouse dark after standby). A permanent status-bar notice shows a service-wide error for
    // as long as it lasts; transient status messages would overwrite it.
    auto* notice=window_.statusBar()->findChild<QLabel*>("serviceNotice");
    if(!notice){notice=new QLabel;notice->setObjectName("serviceNotice");notice->setTextFormat(Qt::PlainText);window_.statusBar()->addWidget(notice,1);}
    notice->setText(error);notice->setToolTip(error);notice->setVisible(!error.isEmpty());
    // A service-wide error counts as an error but is no faulted channel: idle channels stay idle (CodeRabbit review 2026-11-18).
    window_.setOutputState(QString("%1 active / %2 configured / %3 errors / %4 faulted").arg(running).arg(outputs_->topLevelItemCount()).arg(errors+(!error.isEmpty()?1:0)).arg(errors));
}
void ServicePanel::poll() {
    if(!active_)return;
    if(pending_.valid()) {
        if(pending_.wait_for(std::chrono::milliseconds(0))!=std::future_status::ready)return;
        try {
            const auto reply=pending_.get();
            if(!ready_) {
                // Service playback is authoritative on reopen; explicitly recovered unsaved edits take precedence.
                if(!preferLocal_ && !dirty_ && reply.value("project").isObject()) {
                    QScopedValueRollback<bool> restoring(restoring_,true);
                    QTemporaryFile file;if(!file.open())throw std::runtime_error("Cannot restore service snapshot");
                    const auto bytes=QJsonDocument(reply.value("project").toObject()).toJson();
                    if(file.write(bytes)!=bytes.size() || !file.flush())throw std::runtime_error("Cannot stage service snapshot");
                    window_.loadProject(file.fileName(),true,{},reply.value("componentDefinitions").toArray());window_.document().history()->setClean();
                    window_.findChild<QDoubleSpinBox*>("previewTime")->setValue(reply.value("seconds").toDouble());
                    window_.findChild<QPushButton*>("playPreview")->setChecked(reply.value("playing").toBool());
                } else if(!window_.document().state().assignments.isEmpty())dirty_=true;
                ready_=true;
                // Discover integrated devices only after restoring the canonical project.
                QTimer::singleShot(0,window_.findChild<QPushButton*>("refreshHeaders"),&QPushButton::click);
            }
            display(reply);
        } catch(const std::exception& e){
            // Keep unsynchronized edits pending after transport failure; never label them applied.
            if(pendingCommand_.value("op")=="project")dirty_=true;
            if(pendingCommand_.value("op")=="playback")clockDirty_=true;
            status_->setText(QString::fromUtf8(e.what()));window_.setOutputState("Service unavailable");
        }
        nextPoll_=QDateTime::currentMSecsSinceEpoch()+1000;
    }
    const auto now=QDateTime::currentMSecsSinceEpoch();QJsonObject next;
    if(!commands_.isEmpty())next=commands_.takeFirst();
    // Edits stream while they happen, one in flight at a time and at most every 80 ms. Waiting for
    // 200 ms of quiet plus the 1 s status interval sent nothing during a drag (owner review 2026-11-17).
    else if(ready_ && dirty_ && now-projectSentAt_>=80) {
        // Measured from dispatch, after staging: a slow staging must not shorten the next gap (CodeRabbit review 2026-11-18).
        try{next=project();dirty_=false;clockDirty_=false;projectSentAt_=QDateTime::currentMSecsSinceEpoch();}
        catch(const std::exception& e){status_->setText(QString::fromUtf8(e.what()));dirty_=false;return;}
    } else if(ready_ && clockDirty_) {
        next=command("playback");next["playing"]=window_.findChild<QPushButton*>("playPreview")->isChecked();next["seconds"]=window_.previewSeconds();clockDirty_=false;
    } else if(now>=nextPoll_)next=command(ready_?"status":"snapshot");
    else return;
    pendingCommand_=next;pending_=std::async(std::launch::async,[next,rpc=rpc_]{return rpc(next);});
}

void ServicePanel::flushOnClose() {
    timer_.stop();active_=false;QStringList errors;
    const auto apply=[this,&errors](const QJsonObject& request) {
        try{display(rpc_(request));return true;}catch(const std::exception& e){errors.append(QString::fromUtf8(e.what()));return false;}
    };
    if(pending_.valid()) {
        try{display(pending_.get());}catch(const std::exception& e){
            errors.append(QString::fromUtf8(e.what()));if(pendingCommand_.value("op")=="project")dirty_=true;
        }
    }
    // A failed project upload must not suppress a queued explicit Stop output command.
    for(const auto& request:commands_)apply(request);commands_.clear();
    if(ready_ && dirty_) {
        try{dirty_=!apply(project());}catch(const std::exception& e){errors.append(QString::fromUtf8(e.what()));}
    } else if(ready_ && clockDirty_) {
        auto request=command("playback");request["playing"]=window_.findChild<QPushButton*>("playPreview")->isChecked();request["seconds"]=window_.previewSeconds();apply(request);
    }
    if(!errors.isEmpty()) {
        errors.removeDuplicates();
        QMessageBox::warning(&window_,"Lighting service synchronization failed",QString("Some editor commands were not confirmed. The last confirmed service state remains active.\n")+errors.join("\n"));
    }
}
