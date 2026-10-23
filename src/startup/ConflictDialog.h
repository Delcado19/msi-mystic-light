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
    void refresh();
    void terminate(bool all);
    void updateButtons();
    IConflictProcesses& processes_;
    QVector<ConflictProcess> conflicts_;
    QTableWidget* table_;
    QPushButton *terminate_, *terminateAll_, *ignore_;
    QLabel* status_;
    bool busy_ = false;
};
