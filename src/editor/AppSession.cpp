#include "AppSession.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QStatusBar>
#include <stdexcept>

AppSession::AppSession(LightingWindow& window,QSettings& settings,QString recoveryFile)
    : QObject(&window), window_(window), settings_(settings), recoveryFile_(std::move(recoveryFile))
{
    timer_.setSingleShot(true); timer_.setInterval(2000);
    connect(&timer_,&QTimer::timeout,this,[this] { writeRecovery(); });
    connect(&window_.document(),&LightingDocument::changed,this,[this] { timer_.start(); });
    connect(window_.document().history(),&QUndoStack::cleanChanged,this,[this](bool clean) {
        if(clean) {
            timer_.stop();
            // Recovery load briefly looks clean before loadProject marks it unsaved; defer deletion too.
            QTimer::singleShot(0,this,[this] {
                if(window_.document().history()->isClean()) { QFile::remove(recoveryFile_); persist(); }
            });
        }
    });
    connect(&window_,&LightingWindow::closing,this,[this] {
        timer_.stop(); persist(); QFile::remove(recoveryFile_);
    });
}
void AppSession::persist()
{
    settings_.setValue("window/geometry",window_.saveGeometry());
    settings_.setValue("window/state",window_.saveState(1));
    settings_.setValue("project/path",window_.projectFile()); settings_.sync();
}
bool AppSession::writeRecovery()
{
    if(window_.document().history()->isClean()) return true;
    try {
        if(!QDir().mkpath(QFileInfo(recoveryFile_).absolutePath())) throw std::runtime_error("Cannot create recovery directory");
        window_.document().save(recoveryFile_,false); persist(); return true;
    } catch(const std::exception& error) {
        window_.statusBar()->showMessage("Recovery save failed: "+QString::fromUtf8(error.what()),10000); return false;
    }
}
void AppSession::restore(const std::function<bool()>& recover)
{
    window_.restoreGeometry(settings_.value("window/geometry").toByteArray());
    // Version 1 drops the old bottom-output-dock layout while retaining projects and window geometry.
    window_.restoreState(settings_.value("window/state").toByteArray(),1);
    const auto original=settings_.value("project/path").toString();
    const bool available=QFileInfo::exists(recoveryFile_);
    const bool recovering=available && recover && recover();
    if(available && !recovering) QFile::remove(recoveryFile_);
    const auto path=recovering ? recoveryFile_ : original;
    if(path.isEmpty() || !QFileInfo::exists(path)) return;
    try { window_.loadProject(path,recovering,original); }
    catch(const std::exception& error) { QMessageBox::warning(&window_,"Restore failed",QString::fromUtf8(error.what())); }
}
