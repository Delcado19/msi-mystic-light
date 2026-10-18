// Thin Qt wrapper around QSystemTrayIcon/QMenu for the app's tray icon shell
// (v1 skeleton plan, Task 10). Only a "Quit" action exists for now; Task 12
// wires this into the full application (adding more menu items then).
#pragma once

#include <QMenu>
#include <QObject>
#include <QSystemTrayIcon>

class TrayApp : public QObject
{
    Q_OBJECT
public:
    explicit TrayApp(QObject* parent = nullptr);

    // Shows the tray icon. Returns false if QSystemTrayIcon::isSystemTrayAvailable()
    // is false (e.g. no system tray present), without attempting to show it.
    bool start();

    // Exposed for tests: lets tests inspect/trigger menu actions without
    // needing a visible desktop tray icon (start() is not required/called).
    QMenu* contextMenu() const;

signals:
    void quitRequested();

private:
    QSystemTrayIcon trayIcon_;
    QMenu menu_;
};
