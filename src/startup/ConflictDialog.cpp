#include "ConflictDialog.h"
#include <QAbstractItemView>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <stdexcept>
#include <algorithm>

ConflictDialog::ConflictDialog(IConflictProcesses& processes, QWidget* parent)
    : QDialog(parent), processes_(processes)
{
    setWindowTitle("Potential lighting conflicts");
    resize(940, 430);
    auto* layout = new QVBoxLayout(this);
    auto* explanation = new QLabel("These programs may control the same lighting devices. Select processes to stop, "
        "or choose Ignore to continue. Terminate requests a normal close, then forces remaining selected processes "
        "after a short wait. Use the separate administrator action for the SignalRGB service; it never kills the service process. "
        "Its automatic startup and scheduled tasks are unchanged. Unsaved work in those programs may be lost.", this);
    explanation->setWordWrap(true);
    layout->addWidget(explanation);
    table_ = new QTableWidget(0, 4, this);
    table_->setObjectName("conflictTable");
    table_->setHorizontalHeaderLabels({"Executable", "PID", "Path", "Reason"});
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(table_);
    status_ = new QLabel(this);
    status_->setObjectName("conflictStatus");
    status_->setWordWrap(true);
    status_->setTextFormat(Qt::PlainText);
    layout->addWidget(status_);
    auto* buttons = new QHBoxLayout;
    auto button = [&](const QString& text, const QString& name) {
        auto* result = new QPushButton(text, this);
        result->setObjectName(name);
        result->setAutoDefault(false);
        buttons->addWidget(result);
        return result;
    };
    terminateAll_ = button("Terminate all", "terminateAll");
    terminate_ = button("Terminate", "terminateSelected");
    adminStop_ = button("Stop SignalRGB service (administrator)", "stopSignalRgbServiceAdmin");
    buttons->addStretch();
    auto* rescan = button("Rescan", "rescan");
    ignore_ = button("Ignore", "ignore");
    auto* cancel = button("Cancel", "cancel");
    cancel->setDefault(true);
    layout->addLayout(buttons);
    connect(terminate_, &QPushButton::clicked, this, [this] { terminate(false); });
    connect(terminateAll_, &QPushButton::clicked, this, [this] { terminate(true); });
    connect(adminStop_, &QPushButton::clicked, this, [this] { stopSignalRgbService(); });
    connect(rescan, &QPushButton::clicked, this, [this] { if (!busy_) refresh(); });
    connect(ignore_, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(table_, &QTableWidget::itemSelectionChanged, this, [this] { updateButtons(); });
    refresh();
}

bool ConflictDialog::refresh(bool acceptEmpty)
{
    try {
        conflicts_ = processes_.scan();
        table_->setRowCount(0);
        for (const auto& process : conflicts_) {
            const int row = table_->rowCount();
            table_->insertRow(row);
            const QStringList values{process.executable, QString::number(process.pid),
                process.path.isEmpty() ? "Unavailable" : process.path,
                process.reason + (process.manualStop ? " (manual service stop required)" : "")};
            for (int col = 0; col < values.size(); ++col)
                table_->setItem(row, col, new QTableWidgetItem(values[col]));
        }
        adminServicePid_=0;QString serviceError;
        if(std::any_of(conflicts_.cbegin(),conflicts_.cend(),[](const auto& p){return isSignalRgbService(p.executable);})) {
            ConflictService service;serviceError=processes_.querySignalRgbService(service);
            if(serviceError.isEmpty())serviceError=signalRgbAdminStopError(service,conflicts_);
            if(serviceError.isEmpty())adminServicePid_=service.pid;
        }
        if (conflicts_.isEmpty() && acceptEmpty) {
            QTimer::singleShot(0, this, &QDialog::accept);
            updateButtons();
            return true;
        }
        status_->setText("Select one or more rows. Remaining or restarted processes stay listed after a rescan."+
            (serviceError.isEmpty()?QString{}:"\nAdministrator service stop unavailable: "+serviceError));
    } catch (const std::exception& error) {
        conflicts_.clear();
        adminServicePid_=0;
        table_->setRowCount(0);
        status_->setText(QString("Conflict scan failed: %1. Retry, Ignore explicitly, or Cancel.").arg(error.what()));
        updateButtons();
        return false;
    }
    updateButtons();
    return true;
}

void ConflictDialog::updateButtons()
{
    terminate_->setEnabled(!busy_ && !table_->selectionModel()->selectedRows().isEmpty());
    terminateAll_->setEnabled(!busy_ && !conflicts_.isEmpty());
    ignore_->setEnabled(!busy_);
    adminStop_->setEnabled(!busy_ && adminServicePid_!=0);
}

void ConflictDialog::stopSignalRgbService()
{
    if(busy_ || !adminServicePid_)return;
    const auto originalPid=adminServicePid_;busy_=true;updateButtons();
    status_->setText("Requesting administrator approval to stop SignalRGB service...");
    QString error;
    try {error=processes_.requestSignalRgbAdminStop(originalPid);}
    catch(const std::exception& e){error=QString::fromUtf8(e.what());}
    if(!error.isEmpty()) {
        busy_=false;
        if(refresh(false))status_->setText(error);
        else status_->setText(status_->text()+"\n"+error);
        return;
    }
    verifySignalRgbService(originalPid,300);
}

void ConflictDialog::verifySignalRgbService(quint32 originalPid,int checksLeft,int stableChecks)
{
    if(result()==QDialog::Rejected && !isVisible()){busy_=false;return;}
    bool pending=false;QString error;
    ConflictService service;
    try {
        error=processes_.pollSignalRgbAdminStop(pending);
        if(error.isEmpty())error=processes_.querySignalRgbService(service);
    } catch(const std::exception& e){error=QString::fromUtf8(e.what());}
    if(!refresh(false)){busy_=false;updateButtons();if(!error.isEmpty())status_->setText(status_->text()+"\n"+error);return;}
    const bool originalPresent=std::any_of(conflicts_.cbegin(),conflicts_.cend(),[=](const auto& p){return p.pid==originalPid && isSignalRgbService(p.executable);});
    if(error.isEmpty() && service.pid && service.pid!=originalPid)error="SignalRGB service restarted; no automatic second stop was sent";
    const bool stopped=!pending && service.state==ConflictService::Stopped && !originalPresent;
    stableChecks=stopped?stableChecks+1:0;
    if(error.isEmpty() && stableChecks>=6) {
        busy_=false;updateButtons();status_->setText("SignalRGB service stopped. Automatic startup and scheduled tasks are unchanged.");
        if(conflicts_.isEmpty())accept();return;
    }
    if(error.isEmpty() && checksLeft>0) {
        status_->setText("Waiting for SignalRGB service to stop and remain stopped...");
        QTimer::singleShot(100,this,[this,originalPid,checksLeft,stableChecks]{verifySignalRgbService(originalPid,checksLeft-1,stableChecks);});return;
    }
    busy_=false;updateButtons();
    status_->setText(error.isEmpty()?"SignalRGB stop verification timed out. The request may still complete; rescan before retrying.":error);
}

void ConflictDialog::terminate(bool all)
{
    QVector<ConflictProcess> targets;
    if (all) targets = conflicts_;
    else for (const auto& row : table_->selectionModel()->selectedRows()) targets.append(conflicts_[row.row()]);
    if (targets.isEmpty() || busy_) return;
    busy_ = true;
    updateButtons();
    QStringList errors;
    QVector<ConflictProcess> closable;
    for (const auto& process : targets) {
        const auto error = processes_.stop(process, false);
        if (error.isEmpty()) closable.append(process);
        else errors.append(process.executable + ": " + error);
    }
    status_->setText("Waiting for selected programs to close...");
    // The context cancels the delayed force step if the user cancels/closes the dialog.
    QTimer::singleShot(1200, this, [this, closable, errors]() mutable {
        if (result() == QDialog::Rejected && !isVisible()) { busy_ = false; return; }
        for (const auto& process : closable) {
            const auto error = processes_.stop(process, true);
            if (!error.isEmpty()) errors.append(process.executable + ": " + error);
        }
        const bool service = std::any_of(closable.begin(), closable.end(), [](const auto& p) { return isSignalRgbService(p.executable); });
        verifyStopped(closable, errors, service ? 300 : 20);
    });
}

void ConflictDialog::verifyStopped(const QVector<ConflictProcess>& targets, QStringList errors, int checksLeft)
{
    if (result() == QDialog::Rejected && !isVisible()) { busy_ = false; return; }
    // A transient empty scan must not hide a failed stop or a replacement service.
    if (!refresh(false)) {
        busy_ = false;
        updateButtons();
        if (!errors.isEmpty()) status_->setText(status_->text() + "\n" + errors.join('\n'));
        return;
    }
    for (const auto& target : targets) if (isSignalRgbService(target.executable)) {
        const auto error = processes_.stop(target, true);
        const auto message = target.executable + ": " + error;
        if (!error.isEmpty() && !errors.contains(message)) errors.append(message);
    }
    QStringList remaining;
    for (const auto& current : conflicts_) for (const auto& target : targets)
        if (current.pid == target.pid && current.started == target.started &&
            current.path.compare(target.path, Qt::CaseInsensitive) == 0)
            remaining.append(current.executable);
    // TerminateProcess is asynchronous. Recheck the original identities without
    // another click or automatically terminating newly started replacements.
    if (!remaining.isEmpty() && checksLeft > 0) {
        status_->setText("Waiting for selected programs to exit..." +
            (errors.isEmpty() ? QString{} : "\n" + errors.join('\n')));
        QTimer::singleShot(100, this, [this, targets, errors, checksLeft] {
            verifyStopped(targets, errors, checksLeft - 1);
        });
        return;
    }
    busy_ = false;
    updateButtons();
    if (!remaining.isEmpty()) errors.append("Still running after termination: " + remaining.join(", ") + ". Retry or stop manually.");
    if (!errors.isEmpty()) status_->setText(errors.join('\n'));
    else if(conflicts_.isEmpty())accept();
}
