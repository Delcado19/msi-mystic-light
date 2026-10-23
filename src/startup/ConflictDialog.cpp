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

ConflictDialog::ConflictDialog(IConflictProcesses& processes, QWidget* parent)
    : QDialog(parent), processes_(processes)
{
    setWindowTitle("Potential lighting conflicts");
    resize(940, 430);
    auto* layout = new QVBoxLayout(this);
    auto* explanation = new QLabel("These programs may control the same lighting devices. Select processes to stop, "
        "or choose Ignore to continue. Terminate requests a normal close, then forces remaining selected processes "
        "after a short wait. Unsaved work in those programs may be lost.", this);
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
    buttons->addStretch();
    auto* rescan = button("Rescan", "rescan");
    ignore_ = button("Ignore", "ignore");
    auto* cancel = button("Cancel", "cancel");
    cancel->setDefault(true);
    layout->addLayout(buttons);
    connect(terminate_, &QPushButton::clicked, this, [this] { terminate(false); });
    connect(terminateAll_, &QPushButton::clicked, this, [this] { terminate(true); });
    connect(rescan, &QPushButton::clicked, this, [this] { if (!busy_) refresh(); });
    connect(ignore_, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(table_, &QTableWidget::itemSelectionChanged, this, [this] { updateButtons(); });
    refresh();
}

void ConflictDialog::refresh()
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
        if (conflicts_.isEmpty()) {
            QTimer::singleShot(0, this, &QDialog::accept);
            return;
        }
        status_->setText("Select one or more rows. Remaining or restarted processes stay listed after a rescan.");
    } catch (const std::exception& error) {
        conflicts_.clear();
        table_->setRowCount(0);
        status_->setText(QString("Conflict scan failed: %1. Retry, Ignore explicitly, or Cancel.").arg(error.what()));
    }
    updateButtons();
}

void ConflictDialog::updateButtons()
{
    terminate_->setEnabled(!busy_ && !table_->selectionModel()->selectedRows().isEmpty());
    terminateAll_->setEnabled(!busy_ && !conflicts_.isEmpty());
    ignore_->setEnabled(!busy_);
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
        busy_ = false;
        refresh();
        if (!errors.isEmpty()) status_->setText(errors.join('\n'));
    });
}
