#include "ServicePanel.h"
#include "ServiceProtocol.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDoubleSpinBox>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QPushButton>
#include <QMessageBox>
#include <QHeaderView>
#include <QSaveFile>
#include <QScopedValueRollback>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <stdexcept>
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
ServicePanel::ServicePanel(LightingWindow& window,Rpc rpc):QWidget(&window),rpc_(std::move(rpc)),window_(window) {
    if(!rpc_)rpc_=[](const QJsonObject& request){return StudioService::request(request,host());};
    setObjectName("outputPanel");auto* layout=new QVBoxLayout(this);
    auto* note=new QLabel("Windows service owns hardware output. Edits synchronize automatically; closing the editor keeps lighting running.");
    note->setWordWrap(true);layout->addWidget(note);
    auto* buttons=new QHBoxLayout;layout->addLayout(buttons);
    const auto button=[&](const QString& text,const std::function<void()>& action){auto* b=new QPushButton(text);buttons->addWidget(b);connect(b,&QPushButton::clicked,this,action);};
    button("Resume all output",[this]{auto c=command("output");c["enabled"]=true;send(c);});
    button("Stop all output",[this]{stopOutput();});
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
    connect(&window_.document(),&LightingDocument::changed,this,[this]{if(active_ && !restoring_){dirty_=true;changedAt_=QDateTime::currentMSecsSinceEpoch();}});
    connect(window_.findChild<QPushButton*>("playPreview"),&QPushButton::toggled,this,[this]{if(ready_)clockDirty_=true;});
    // Clock value changes during normal playback need no IPC; the service advances its own clock.
    connect(window_.findChild<QDoubleSpinBox*>("previewTime"),&QDoubleSpinBox::editingFinished,this,[this]{if(ready_)clockDirty_=true;});
    connect(&window_,&LightingWindow::closing,this,[this]{flushOnClose();});
    timer_.setInterval(50);connect(&timer_,&QTimer::timeout,this,[this]{poll();});
}
void ServicePanel::activate(bool preferLocal) {preferLocal_=preferLocal;active_=true;send(command("snapshot"));timer_.start();}
void ServicePanel::stopOutput(){auto c=command("output");c["enabled"]=false;send(c);}
void ServicePanel::send(QJsonObject c) {
    if(commands_.size()>=16){status_->setText("Service command queue full; wait for the current operation.");return;}
    commands_.append(c);
}
QJsonObject ServicePanel::project() {
    QTemporaryDir folder;if(!folder.isValid())throw std::runtime_error("Cannot stage editor project");
    const auto path=folder.path()+"/project.json";window_.document().save(path,false);
    QFile input(path);if(!input.open(QIODevice::ReadOnly))throw std::runtime_error("Cannot read staged editor project");
    auto result=command("project");result["project"]=StudioService::decode(input.readAll());
    result["enabled"]=enabled_;result["playing"]=window_.findChild<QPushButton*>("playPreview")->isChecked();result["seconds"]=window_.previewSeconds();return result;
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
    window_.setOutputState(QString("%1 active / %2 configured / %3 errors").arg(running).arg(outputs_->topLevelItemCount()).arg(errors+(!error.isEmpty()?1:0)));
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
                    window_.loadProject(file.fileName(),true,{});window_.document().history()->setClean();
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
    else if(ready_ && dirty_ && now>=nextPoll_ && now-changedAt_>=200) {
        try{next=project();dirty_=false;clockDirty_=false;}
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
