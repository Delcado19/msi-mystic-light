// Thin Qt wrapper around QSystemTrayIcon/QMenu for the app's tray icon shell
// (v1 skeleton plan, Task 10). Task 16 adds "Open Lighting Studio" and a
// profile quick-switch submenu; Task 18 wires openStudioRequested/
// profileSelected into the real LightingStudioWindow/Settings.
#pragma once

#include <QMenu>
#include <QObject>
#include <QSystemTrayIcon>

#include <string>
#include <vector>

class QActionGroup;

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

    // Replaces the profile submenu's contents; called by main.cpp whenever
    // Settings.profiles changes (e.g. after a save in the Studio window).
    // Rebuilds from scratch each call (rather than diffing old/new) so a
    // profile renamed/added/removed elsewhere is always reflected correctly.
    void setAvailableProfiles(const std::vector<std::string>& profileNames, const std::string& activeProfileName);

signals:
    void quitRequested();
    void openStudioRequested();
    void profileSelected(const std::string& profileName);

private:
    QSystemTrayIcon trayIcon_;
    QMenu menu_;
    QMenu* profileMenu_;
    // Exclusive group backing the profile submenu's checkable actions, so
    // checking one entry unchecks the rest (radio-button behavior). Replaced
    // (via deleteLater(), see setAvailableProfiles()) each time the profile
    // list changes.
    QActionGroup* profileGroup_ = nullptr;
};
