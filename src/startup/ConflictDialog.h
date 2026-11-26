#pragma once

#include "ConflictProcesses.h"
#include <QDialog>

class QTableWidget;
class QPushButton;
class QLabel;

class ConflictDialog final : public QDialog {
public:
    explicit ConflictDialog(IConflictProcesses& processes, QWidget* parent = nullptr);
private:
    bool refresh(bool acceptEmpty=true);
    void terminate(bool all);
    void verifyStopped(const QVector<ConflictProcess>& targets, QStringList errors, int checksLeft);
    void updateButtons();
    void stopSignalRgbService();
    void verifySignalRgbService(quint32 originalPid,int checksLeft,int stableChecks=0);
    IConflictProcesses& processes_;
    QVector<ConflictProcess> conflicts_;
    // One table row per program (same image path); each entry holds indices into conflicts_, so a stop
    // still reaches every process of that program (owner review 2026-11-26: ten identical rows).
    QVector<QVector<int>> groups_;
    QTableWidget* table_;
    QPushButton *terminate_, *terminateAll_, *ignore_, *adminStop_;
    QLabel* status_;
    bool busy_ = false;
    quint32 adminServicePid_ = 0;
};
