#include "OutputPanel.h"
#include <QLabel>
#include <QPushButton>
#include <QTreeWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <stdexcept>

namespace {
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
        "Editing or loading a document stops output and requires a new review.");
    note->setWordWrap(true); layout->addWidget(note);
    start_=new QPushButton("Review and start output"); start_->setObjectName("startOutput"); layout->addWidget(start_);
    stop_=new QPushButton("Stop output"); stop_->setObjectName("stopOutput"); layout->addWidget(stop_);
    status_=new QLabel; status_->setWordWrap(true); status_->setObjectName("outputStatus"); layout->addWidget(status_);
    endpoints_=new QTreeWidget; endpoints_->setObjectName("outputEndpoints");
    endpoints_->setHeaderLabels({"Endpoint / profile", "State"});
    endpoints_->header()->setSectionResizeMode(0,QHeaderView::Stretch);
    endpoints_->header()->setSectionResizeMode(1,QHeaderView::ResizeToContents); layout->addWidget(endpoints_);
    connect(start_,&QPushButton::clicked,this,&OutputPanel::start);
    connect(stop_,&QPushButton::clicked,this,[this] { stop("Output stopped. Devices retain their last applied state."); });
    connect(&window_,&LightingWindow::previewFrameChanged,this,&OutputPanel::dispatch);
    connect(&window_.document(),&LightingDocument::changed,this,[this] {
        if(active()) stop("Document changed. Review the new configuration before restarting.");
        refresh();
    });
    connect(&window_,&LightingWindow::closing,this,&OutputPanel::stopOutput);
    refresh();
}
void OutputPanel::stopOutput() { stop("Output stopped. Devices retain their last applied state."); }
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
void OutputPanel::start()
{
    if(authorizing_ || active() || !authorize_) return;
    authorizing_=true; refresh();
    try {
        const auto revision=window_.document().revision();
        const auto frames=prepareOutput(window_.document(),window_.previewFrame(),window_.previewSeconds());
        if(frames.isEmpty()) throw std::invalid_argument("Configure at least one channel assignment first");
        auto requests=authorize_(this,frames);
        if(window_.document().revision()!=revision) throw std::invalid_argument("Document changed during output review; review it again");
        if(requests.isEmpty()) message_="Output review cancelled. No device opened.";
        else {
            connections_.clear(); revision_=revision;
            for(auto& request:requests) {
                connections_.push_back({request.initial.endpoint,request.initial.profileId,std::make_unique<OutputScheduler>(),0});
                auto& connection=connections_.back();
                connect(connection.scheduler.get(),&OutputScheduler::statusChanged,this,[this] {
                    for(const auto& item:connections_) if(item.scheduler->status().state==OutputScheduler::State::Faulted) {
                        stop(item.scheduler->status().error); break;
                    }
                    refresh();
                });
                connection.generation=connection.scheduler->start(std::move(request.factory),std::move(request.present));
                if(!connection.generation || !connection.scheduler->submit(connection.generation,std::move(request.initial)))
                    throw std::runtime_error(connection.scheduler->status().error.isEmpty() ? "Output connection could not be started" : connection.scheduler->status().error.toStdString());
            }
            message_="Output active. Preview time controls software colors; Stop keeps the last device state.";
        }
    } catch(const std::exception& error) { stop(QString::fromUtf8(error.what())); }
    authorizing_=false; refresh();
}
void OutputPanel::dispatch(const QImage& frame)
{
    if(!active()) return;
    // A window refresh can emit a new frame before this panel's document-changed slot runs.
    if(window_.document().revision()!=revision_) { stop("Document changed. Output stopped before submitting its new frame."); return; }
    try {
        const auto frames=prepareOutput(window_.document(),frame,window_.previewSeconds());
        for(auto& connection:connections_) {
            const auto state=connection.scheduler->status().state;
            if(state!=OutputScheduler::State::Starting && state!=OutputScheduler::State::Running) continue;
            const OutputFrame* found=nullptr;
            for(const auto& candidate:frames) if(candidate.endpoint==connection.endpoint && candidate.profileId==connection.profileId) found=&candidate;
            if(!found || !connection.scheduler->submit(connection.generation,*found)) {
                stop("Output target changed or the connection stopped."); return;
            }
        }
    } catch(const std::exception& error) { stop(QString::fromUtf8(error.what())); }
}
void OutputPanel::refresh()
{
    start_->setEnabled(!authorizing_ && !active() && bool(authorize_) && !window_.document().state().assignments.isEmpty());
    stop_->setEnabled(active()); status_->setText(authorizing_ ? "Reviewing output..." : message_);
    endpoints_->clear();
    for(const auto& connection:connections_) {
        const auto state=connection.scheduler->status();
        auto* item=new QTreeWidgetItem(endpoints_,{connection.endpoint+" / "+connection.profileId,stateName(state.state)});
        item->setToolTip(0,connection.endpoint+" / "+connection.profileId); item->setToolTip(1,state.error);
    }
}
