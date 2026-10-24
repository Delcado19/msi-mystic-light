#include "OutputPanel.h"
#include <QLabel>
#include <QTimer>
#include <QPushButton>
#include <QTreeWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <stdexcept>
#include <QSignalBlocker>
#include <algorithm>

namespace {
QString channelKey(const QString& endpoint, const QString& profile, const QString& channel)
{
    return endpoint+"\n"+profile+"\n"+channel;
}
QString stateName(OutputScheduler::State state)
{
    switch(state) {
    case OutputScheduler::State::Stopped: return "Stopped";
    case OutputScheduler::State::Starting: return "Connecting";
    case OutputScheduler::State::Running: return "Running";
    case OutputScheduler::State::Stopping: return "Stopping";
    case OutputScheduler::State::Faulted: return "Faulted";
    }
    return {};
}
}
OutputPanel::OutputPanel(LightingWindow& window, Authorize authorize, QWidget* parent)
    : QWidget(parent), window_(window), authorize_(std::move(authorize))
{
    setObjectName("outputPanel");
    auto* layout=new QVBoxLayout(this);
    auto* note=new QLabel("Output follows preview time. Use Play preview for animation. "
        "Canvas output starts automatically for configured, detected hardware. Changes update live.");
    note->setWordWrap(true); layout->addWidget(note);
    auto* actions=new QHBoxLayout; layout->addLayout(actions);
    start_=new QPushButton("Resume output"); start_->setObjectName("startOutput"); actions->addWidget(start_);
    stop_=new QPushButton("Stop output"); stop_->setObjectName("stopOutput"); actions->addWidget(stop_); actions->addStretch();
    status_=new QLabel; status_->setWordWrap(true); status_->setObjectName("outputStatus"); layout->addWidget(status_);
    endpoints_=new QTreeWidget; endpoints_->setObjectName("outputEndpoints");
    endpoints_->setMinimumHeight(220);
    endpoints_->setHeaderLabels({"Controller / output", "State"});
    endpoints_->header()->setStretchLastSection(false);
    endpoints_->header()->setSectionResizeMode(0,QHeaderView::Stretch);
    endpoints_->header()->setSectionResizeMode(1,QHeaderView::ResizeToContents); layout->addWidget(endpoints_,1);
    automatic_=new QTimer(this); automatic_->setInterval(2000);
    connect(automatic_,&QTimer::timeout,this,[this] { if(!active() && !window_.document().state().assignments.isEmpty()) start(); });
    auto* inventory=window_.findChild<QTreeWidget*>("layoutComponents");
    connect(endpoints_,&QTreeWidget::currentItemChanged,this,[this,inventory] {
        updateControls();
        const auto* output=endpoints_->currentItem();
        if(!output || output->data(0,Qt::UserRole+2).toString().isEmpty()) return;
        const auto* current=inventory->currentItem();
        const auto currentHeader=current ? (current->parent() ? current->parent() : current)->data(0,Qt::UserRole+1).toMap() : QVariantMap{};
        for(int i=0;i<inventory->topLevelItemCount();++i) {
            auto* header=inventory->topLevelItem(i); const auto data=header->data(0,Qt::UserRole+1).toMap();
            if(data.value("path")==output->data(0,Qt::UserRole) && data.value("profileId")==output->data(0,Qt::UserRole+1) &&
                data.value("channelId")==output->data(0,Qt::UserRole+2) && data!=currentHeader) inventory->setCurrentItem(header);
        }
    });
    const auto followInventory=[this,inventory] {
        const auto* current=inventory->currentItem(); if(!current) return;
        const auto data=(current->parent() ? current->parent() : current)->data(0,Qt::UserRole+1).toMap();
        for(int i=0;i<endpoints_->topLevelItemCount();++i) for(int j=0;j<endpoints_->topLevelItem(i)->childCount();++j) {
            auto* row=endpoints_->topLevelItem(i)->child(j);
            if(data.value("path")==row->data(0,Qt::UserRole) && data.value("profileId")==row->data(0,Qt::UserRole+1) &&
                data.value("channelId")==row->data(0,Qt::UserRole+2)) endpoints_->setCurrentItem(row);
        }
    };
    connect(inventory,&QTreeWidget::currentItemChanged,this,followInventory);
    connect(window_.findChild<LightingCanvas*>("layoutCanvas"),&LightingCanvas::selectionChanged,this,followInventory);
    connect(start_,&QPushButton::clicked,this,[this] {
        for(const auto& key:selectedChannels()) pausedChannels_.remove(key);
        const bool connected=active();
        automatic_->start(); start(true);
        // New connections already receive their initial frame; existing paused channels need a fresh submission.
        if(connected) dispatch(window_.previewFrame());
        refresh();
    });
    connect(stop_,&QPushButton::clicked,this,[this] {
        for(const auto& key:selectedChannels()) pausedChannels_.insert(key);
        // Keep the shared controller open; pausing one header must not stop its peers.
        message_="Selected output paused. Other outputs keep running; devices retain their last applied state.";
        dispatch(window_.previewFrame()); refresh();
    });
    connect(&window_,&LightingWindow::hardwareDiscovered,this,&OutputPanel::refresh);
    connect(&window_,&LightingWindow::previewFrameChanged,this,&OutputPanel::dispatch);
    connect(&window_.document(),&LightingDocument::changed,this,[this] {
        refresh();
        if(automatic_->isActive()) QTimer::singleShot(0,this,[this] { if(automatic_->isActive() && !window_.document().state().assignments.isEmpty()) start(); });
    });
    connect(&window_,&LightingWindow::closing,this,&OutputPanel::stopOutput);
    connect(&window_,&LightingWindow::projectLoading,this,[this] { stop("Loading project..."); });
    refresh();
}
void OutputPanel::enableAutomaticOutput()
{
    pausedChannels_.clear(); automatic_->start();
    if(!window_.document().state().assignments.isEmpty()) start();
}
void OutputPanel::stopOutput() { automatic_->stop(); stop("Output stopped. Devices retain their last applied state."); }
bool OutputPanel::active() const
{
    for(const auto& connection:connections_) {
        const auto state=connection.scheduler->status().state;
        if(state==OutputScheduler::State::Starting || state==OutputScheduler::State::Running || state==OutputScheduler::State::Stopping) return true;
    }
    return false;
}
void OutputPanel::stop(const QString& reason)
{
    if(stopping_) return;
    stopping_=true; message_=reason;
    for(auto& connection:connections_) connection.scheduler->stop();
    stopping_=false; refresh();
}
void OutputPanel::start(bool includeMissing)
{
    if(authorizing_ || (!includeMissing && active()) || !authorize_) return;
    authorizing_=true; refresh();
    try {
        const auto revision=window_.document().revision();
        const auto placements=window_.previewPlacements();
        const auto frames=prepareOutput(window_.document(),window_.previewFrame(),window_.previewSeconds(),&placements);
        if(frames.isEmpty()) throw std::invalid_argument("Configure at least one channel assignment first");
        auto pending=frames;
        pending.removeIf([this](const auto& frame) {
            for(const auto& connection:connections_) if(connection.approved.endpoint==frame.endpoint &&
                connection.approved.profileId==frame.profileId) {
                const auto state=connection.scheduler->status().state;
                if(state==OutputScheduler::State::Starting || state==OutputScheduler::State::Running || state==OutputScheduler::State::Stopping) return true;
            }
            return enabledFrame(frame).channels.isEmpty();
        });
        if(pending.isEmpty()) { message_="Output state updated. Running channels follow the canvas."; authorizing_=false; refresh(); return; }
        auto requests=authorize_(this,pending);
        if(window_.document().revision()!=revision) throw std::invalid_argument("Document changed during connection preparation; retrying");
        if(requests.isEmpty()) message_="No configured hardware available. Waiting for a connection.";
        else {
            configured_=frames;
            for(auto& request:requests) {
                connections_.erase(std::remove_if(connections_.begin(),connections_.end(),[&](const auto& old) {
                    return old.approved.endpoint==request.initial.endpoint && old.approved.profileId==request.initial.profileId;
                }),connections_.end());
                connections_.push_back({request.initial,std::make_unique<OutputScheduler>(),0});
                auto& connection=connections_.back();
                connect(connection.scheduler.get(),&OutputScheduler::statusChanged,this,[this] {
                    for(const auto& item:connections_) if(item.scheduler->status().state==OutputScheduler::State::Faulted) {
                        message_=item.scheduler->status().error; break;
                    }
                    refresh();
                });
                connection.generation=connection.scheduler->start(std::move(request.factory),std::move(request.present));
                if(!connection.generation || !connection.scheduler->submit(connection.generation,enabledFrame(std::move(request.initial)))) {
                    connection.scheduler->stop();
                    throw std::runtime_error(connection.scheduler->status().error.isEmpty() ? "Output connection could not be started" : connection.scheduler->status().error.toStdString());
                }
            }
            // Starting an animated output from a paused editor must not freeze the device at its initial frame.
            for(const auto& layer:window_.document().state().layers) if(layer.visible &&
                (layer.spin!=0 || layer.pulse!=0 || layer.hueShift!=0 || layer.driftX!=0 || layer.driftY!=0 ||
                 layer.zoom!=0 || layer.warp!=0 || layer.breathe!=0 || layer.cycleRate!=0 || isParticle(layer.effect))) {
                window_.findChild<QPushButton*>("playPreview")->setChecked(true); break;
            }
            message_="Output active. Effect edits and preview time update software channels live.";
        }
    } catch(const std::exception& error) { message_=QString::fromUtf8(error.what()); }
    authorizing_=false; refresh();
}
void OutputPanel::dispatch(const QImage& frame)
{
    if(!active()) return;
    try {
        const auto placements=window_.previewPlacements();
        const auto frames=prepareOutput(window_.document(),frame,window_.previewSeconds(),&placements);
        QVector<const OutputFrame*> checked;
        bool changed=frames.size()!=configured_.size();
        for(int i=0;!changed && i<frames.size();++i) changed=!sameOutputScope(configured_[i],frames[i]);
        if(changed) {
            // Rebuild channel capacities/native commands atomically; live edits need no approval dialog.
            stop("Updating hardware configuration...");
            if(automatic_->isActive()) QTimer::singleShot(0,this,[this] { if(automatic_->isActive() && !window_.document().state().assignments.isEmpty()) start(); });
            return;
        }
        // Validate the complete live scope before submitting any endpoint.
        for(const auto& connection:connections_) {
            const OutputFrame* found=nullptr;
            for(const auto& candidate:frames) if(sameOutputScope(connection.approved,candidate)) { found=&candidate; break; }
            if(!found) { stop("Output endpoint configuration changed."); return; }
            checked.append(found);
        }
        for(int i=0;i<int(connections_.size());++i) {
            auto& connection=connections_[i]; const auto state=connection.scheduler->status().state;
            if(state!=OutputScheduler::State::Starting && state!=OutputScheduler::State::Running) continue;
            if(!connection.scheduler->submit(connection.generation,enabledFrame(*checked[i]))) {
                connection.scheduler->stop(); message_="Output connection stopped."; refresh();
            }
        }
    } catch(const std::exception& error) { stop(QString::fromUtf8(error.what())); }
}
OutputFrame OutputPanel::enabledFrame(OutputFrame frame) const
{
    frame.channels.removeIf([&](const auto& channel) {
        return pausedChannels_.contains(channelKey(frame.endpoint,frame.profileId,channel.channelId));
    });
    return frame;
}
QStringList OutputPanel::selectedChannels() const
{
    QStringList result;
    const auto* item=endpoints_->currentItem(); if(!item) return result;
    for(const auto& assignment:window_.document().state().assignments)
        if(assignment.endpoint==item->data(0,Qt::UserRole).toString() && assignment.profileId==item->data(0,Qt::UserRole+1).toString() &&
            (item->data(0,Qt::UserRole+2).toString().isEmpty() || assignment.channelId==item->data(0,Qt::UserRole+2).toString()))
            result.append(channelKey(assignment.endpoint,assignment.profileId,assignment.channelId));
    return result;
}
void OutputPanel::updateControls()
{
    bool resume=false, pause=false;
    for(const auto& key:selectedChannels()) {
        bool running=false;
        for(const auto& connection:connections_) for(const auto& channel:connection.approved.channels)
            if(channelKey(connection.approved.endpoint,connection.approved.profileId,channel.channelId)==key) {
                const auto state=connection.scheduler->status().state;
                running=state==OutputScheduler::State::Running || state==OutputScheduler::State::Starting;
            }
        if(pausedChannels_.contains(key) || !running) resume=true;
        if(running && !pausedChannels_.contains(key)) pause=true;
    }
    start_->setEnabled(!authorizing_ && bool(authorize_) && resume);
    stop_->setEnabled(!authorizing_ && pause);
    start_->setToolTip("Resume the selected output, or all configured outputs of the selected controller");
    stop_->setToolTip("Pause the selected output; devices retain their last color. Other headers keep running");
}
void OutputPanel::refresh()
{
    status_->setText(authorizing_ ? "Connecting output..." : message_);
    const auto* selected=endpoints_->currentItem();
    const QString selectedKey=selected ? channelKey(selected->data(0,Qt::UserRole).toString(),selected->data(0,Qt::UserRole+1).toString(),selected->data(0,Qt::UserRole+2).toString()) : QString{};
    const QSignalBlocker block(endpoints_); endpoints_->clear();
    QVector<QPair<QString,QString>> devices;
    const auto append=[&](const QString& endpoint,const QString& profile) {
        const QPair<QString,QString> key{endpoint,profile}; if(!devices.contains(key)) devices.append(key);
    };
    for(const auto& candidate:window_.discoveredHardware()) append(candidate.device.path,candidate.profileId);
    for(const auto& assignment:window_.document().state().assignments) append(assignment.endpoint,assignment.profileId);
    bool anyRunning=false, anyPaused=false, anyStopped=false, faulted=false, starting=false;
    QTreeWidgetItem* restore=nullptr;
    for(const auto& device:devices) {
        const auto& profile=assignmentProfile(window_.document().profiles(),device.second);
        QString connectionState="Stopped", error;
        for(const auto& connection:connections_) if(connection.approved.endpoint==device.first && connection.approved.profileId==device.second) {
            const auto status=connection.scheduler->status(); connectionState=stateName(status.state); error=status.error;
        }
        auto* parent=new QTreeWidgetItem(endpoints_,{profile.name,connectionState}); parent->setExpanded(true);
        parent->setData(0,Qt::UserRole,device.first); parent->setData(0,Qt::UserRole+1,device.second);
        parent->setToolTip(0,device.first+" / "+device.second); parent->setToolTip(1,error);
        if(channelKey(device.first,device.second,{})==selectedKey) restore=parent;
        bool parentRunning=false,parentStopped=false;
        for(const auto& channel:profile.channels) {
            const ChannelAssignment* assignment=nullptr;
            for(const auto& item:window_.document().state().assignments) if(item.endpoint==device.first && item.profileId==device.second && item.channelId==channel.id) assignment=&item;
            const auto key=channelKey(device.first,device.second,channel.id);
            const bool paused=pausedChannels_.contains(key);
            const auto state=!assignment ? QString("Not configured") : paused ? QString("Stopped") : connectionState;
            auto* row=new QTreeWidgetItem(parent,{channel.id,state});
            row->setData(0,Qt::UserRole,device.first); row->setData(0,Qt::UserRole+1,device.second); row->setData(0,Qt::UserRole+2,channel.id);
            row->setToolTip(0,channel.direct ? "Supports canvas output" : "Native lighting only; this profile does not support canvas output"); row->setToolTip(1,error);
            if(assignment) {
                row->setText(0,channel.id+QString(" / %1 components").arg(assignment->placements.size()));
                parentRunning|=state=="Running"; parentStopped|=paused || state=="Stopped";
                anyRunning|=state=="Running"; anyPaused|=paused; anyStopped|=state=="Stopped"; faulted|=state=="Faulted"; starting|=state=="Connecting";
            }
            if(key==selectedKey) restore=row;
        }
        if(parentRunning && parentStopped) parent->setText(1,"Partly running");
        else if(parentStopped && !parentRunning && connectionState=="Running") parent->setText(1,"Stopped");
    }
    if(restore) endpoints_->setCurrentItem(restore);
    else if(endpoints_->topLevelItemCount()) endpoints_->setCurrentItem(endpoints_->topLevelItem(0));
    updateControls();
    emit outputStateChanged(anyRunning ? (anyPaused || anyStopped || faulted ? "Partly running" : "Running") : starting ? "Connecting" : faulted ? "Faulted" : "Stopped");
}
