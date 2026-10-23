#pragma once
#include "LightingWindow.h"
#include <QSettings>
#include <QTimer>

// Recovery is separate from the user's project and never marks an edit as saved.
class AppSession : public QObject {
public:
    AppSession(LightingWindow& window, QSettings& settings, QString recoveryFile);
    void restore(const std::function<bool()>& recover);
    bool writeRecovery();
private:
    void persist();
    LightingWindow& window_;
    QSettings& settings_;
    QString recoveryFile_;
    QTimer timer_;
};
